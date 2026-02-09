#include "services/util/SesameWindowed.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"

namespace services
{
    namespace
    {
        // This reader is used to put an extra '4' in front of a message, in order to exactly calculate the encoded size
        // Only ExtractContiguousRange() and Available() are used by SesameCobs, so only those methods are implemented here.
        class ExtraCharacterReader
            : public infra::LimitedStreamReader
        {
        public:
            using infra::LimitedStreamReader::LimitedStreamReader;

            void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override
            {
                std::abort();
            }

            uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override
            {
                std::abort();
            }

            infra::ConstByteRange ExtractContiguousRange(std::size_t max) override
            {
                if (extraCharacter != 0)
                    return infra::Head(infra::MakeByteRange(character), std::exchange(extraCharacter, 0));
                else
                    return infra::LimitedStreamReader::ExtractContiguousRange(max);
            }

            infra::ConstByteRange PeekContiguousRange(std::size_t start) override
            {
                std::abort();
            }

            bool Empty() const override
            {
                std::abort();
            }

            std::size_t Available() const override
            {
                return infra::LimitedStreamReader::Available() + extraCharacter;
            }

        private:
            static const char character = '\x4';
            std::size_t extraCharacter = 1;
        };
    }

    SesameWindowed::SesameWindowed(infra::BoundedDeque<uint8_t>& receivedMessage, SesameEncoded& delegate)
        : SesameEncodedObserver(delegate)
        , receivedMessage(receivedMessage)
        , ownBufferSize(static_cast<uint16_t>(SesameEncodedObserver::Subject().MaxSendMessageSize()))
        , releaseWindowSize(static_cast<uint16_t>(SesameEncodedObserver::Subject().WorstCaseMessageSize(sizeof(PacketReleaseWindow))))
        , state(std::in_place_type_t<StateSendingInit>(), *this)
    {
        state->Request();
    }

    void SesameWindowed::RequestSendMessage(std::size_t size)
    {
        state->RequestSendMessage(size);
    }

    std::size_t SesameWindowed::MaxSendMessageSize() const
    {
        assert(initialized);
        return (std::min(ownBufferSize, maxUsableBufferSize) - sizeof(Operation) - releaseWindowSize - SesameEncodedObserver::Subject().WorstCaseMessageSize(sizeof(Operation))) / 2;
    }

    void SesameWindowed::Reset()
    {
        SesameEncodedObserver::Subject().Reset();
        assert(receivedMessageReader == nullptr);
        assert(!readerAccess.Referenced());
        initialized = false;
        otherAvailableWindow = 0;
        maxUsableBufferSize = 0;
        releasedWindow = 0;
        sendInitResponse = false;
        sending = false;
        requestedSendMessageSize.reset();
        state.Emplace<StateSendingInit>(*this);
        state->Request();
    }

    void SesameWindowed::Stop()
    {
        readerAccess.SetAction([]() {});
    }

    void SesameWindowed::Initialized()
    {
        std::abort();
    }

    void SesameWindowed::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        state->SendMessageStreamAvailable(std::move(writer));
    }

    void SesameWindowed::MessageSent(std::size_t encodedSize)
    {
        state->MessageSent(encodedSize);
    }

    void SesameWindowed::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, std::size_t encodedSize)
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        switch (stream.Extract<Operation>())
        {
            case Operation::init:
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                ReceivedInit(otherAvailableWindow);
                sendInitResponse = true;
                ReceivedInitialize();
                break;
            case Operation::initResponse:
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                ReceivedInitResponse(otherAvailableWindow);
                releasedWindow = static_cast<uint16_t>(encodedSize);
                ReceivedInitialize();
                break;
            case Operation::releaseWindow:
                if (initialized)
                {
                    releasedWindow += encodedSize;
                    auto oldOtherAvailableWindow = otherAvailableWindow;
                    otherAvailableWindow += stream.Extract<infra::LittleEndian<uint16_t>>();
                    ReceivedReleaseWindow(oldOtherAvailableWindow, otherAvailableWindow);
                }
                break;
            case Operation::message:
                if (initialized)
                    SaveReceivedMessage(*reader);
                break;
        }

        SetNextState();
    }

    void SesameWindowed::ReceivedInitialize()
    {
        maxUsableBufferSize = otherAvailableWindow;
        initialized = true;
        GetObserver().Initialized();
    }

    void SesameWindowed::SaveReceivedMessage(infra::StreamReader& reader)
    {
        infra::BoundedDequeOutputStream stream(receivedMessage);

        stream << static_cast<uint16_t>(reader.Available());
        while (!reader.Empty())
            stream << reader.ExtractContiguousRange(std::numeric_limits<uint16_t>::max());

        TryForwardReceivedMessage();
    }

    void SesameWindowed::TryForwardReceivedMessage()
    {
        if (!receivedMessageReader && !receivedMessage.empty())
        {
            infra::BoundedDequeInputStream stream(receivedMessage);
            currentReceiveMessageSize = stream.Extract<uint16_t>();
            auto encodedSize = SesameEncodedObserver::Subject().MessageSize(ExtraCharacterReader(stream.Reader(), currentReceiveMessageSize));
            receivedMessage.erase(receivedMessage.begin(), receivedMessage.begin() + 2);

            receivedMessageReader = currentReceiveMessageReader.Emplace(std::in_place, receivedMessage, currentReceiveMessageSize);
            ForwardReceivedMessage(static_cast<uint16_t>(encodedSize));
        }
    }

    void SesameWindowed::ForwardReceivedMessage(uint16_t encodedSize)
    {
        readerAccess.SetAction([this, encodedSize]()
            {
                releasedWindow += encodedSize;
                receivedMessageReader = nullptr;
                receivedMessage.erase(receivedMessage.begin(), receivedMessage.begin() + currentReceiveMessageSize);
                TryForwardReceivedMessage();
                SetNextState();
            });

        ForwardingReceivedMessage(*receivedMessageReader);
        GetObserver().ReceivedMessage(readerAccess.MakeShared(*receivedMessageReader));
    }

    void SesameWindowed::SetNextState()
    {
        if (!sending && initialized)
        {
            if (sendInitResponse)
            {
                if (receivedMessageReader == nullptr)
                    state.Emplace<StateSendingInitResponse>(*this).Request();
            }
            else if (requestedSendMessageSize != std::nullopt && SesameEncodedObserver::Subject().WorstCaseMessageSize(*requestedSendMessageSize + 1) + releaseWindowSize <= otherAvailableWindow)
                state.Emplace<StateSendingMessage>(*this).Request();
            else if (releasedWindow > releaseWindowSize && releaseWindowSize <= otherAvailableWindow)
                state.Emplace<StateSendingReleaseWindow>(*this).Request();
            else
                state.Emplace<StateOperational>(*this);
        }
    }

    SesameWindowed::PacketInit::PacketInit(uint16_t window)
        : window(window)
    {}

    SesameWindowed::PacketInitResponse::PacketInitResponse(uint16_t window)
        : window(window)
    {}

    SesameWindowed::PacketReleaseWindow::PacketReleaseWindow(uint16_t window)
        : window(window)
    {}

    SesameWindowed::State::State(SesameWindowed& communication)
        : communication(communication)
    {}

    void SesameWindowed::State::Request()
    {
        std::abort();
    }

    void SesameWindowed::State::RequestSendMessage(std::size_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void SesameWindowed::State::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        std::abort();
    }

    void SesameWindowed::State::MessageSent(std::size_t encodedSize)
    {
        communication.otherAvailableWindow -= encodedSize;
        communication.sending = false;
        communication.SetNextState();
    }

    SesameWindowed::StateSendingInit::StateSendingInit(SesameWindowed& communication)
        : State(communication)
    {
        communication.sending = true;
    }

    void SesameWindowed::StateSendingInit::Request()
    {
        communication.SesameEncodedObserver::Subject().RequestSendMessage(3);
    }

    void SesameWindowed::StateSendingInit::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingInit(communication.ownBufferSize);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInit(communication.ownBufferSize);
    }

    void SesameWindowed::StateSendingInit::MessageSent(std::size_t encodedSize)
    {
        // Init messages do not count against window size
        communication.sending = false;
        communication.SetNextState();
    }

    SesameWindowed::StateSendingInitResponse::StateSendingInitResponse(SesameWindowed& communication)
        : State(communication)
    {
        communication.sending = true;
    }

    void SesameWindowed::StateSendingInitResponse::Request()
    {
        communication.SesameEncodedObserver::Subject().RequestSendMessage(3);
    }

    void SesameWindowed::StateSendingInitResponse::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingInitResponse(communication.ownBufferSize);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInitResponse(communication.ownBufferSize);

        communication.releasedWindow = 0;
        communication.sendInitResponse = false;
    }

    SesameWindowed::StateOperational::StateOperational(SesameWindowed& communication)
        : State(communication)
    {
        communication.SettingOperational(communication.requestedSendMessageSize, communication.releasedWindow, communication.otherAvailableWindow);
    }

    void SesameWindowed::StateOperational::RequestSendMessage(std::size_t size)
    {
        communication.requestedSendMessageSize = size;
        communication.SetNextState();
    }

    SesameWindowed::StateSendingMessage::StateSendingMessage(SesameWindowed& communication)
        : State(communication)
        , requestedSize(*communication.requestedSendMessageSize + 1)
    {
        communication.sending = true;
    }

    void SesameWindowed::StateSendingMessage::Request()
    {
        communication.SesameEncodedObserver::Subject().RequestSendMessage(requestedSize);
    }

    void SesameWindowed::StateSendingMessage::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingMessage(*writer);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << Operation::message;

        communication.requestedSendMessageSize.reset();
        communication.GetObserver().SendMessageStreamAvailable(std::move(writer));
    }

    void SesameWindowed::StateSendingMessage::MessageSent(std::size_t encodedSize)
    {
        communication.otherAvailableWindow -= encodedSize;

        communication.sending = false;
        communication.SetNextState();
    }

    SesameWindowed::StateSendingReleaseWindow::StateSendingReleaseWindow(SesameWindowed& communication)
        : State(communication)
    {
        communication.sending = true;
    }

    void SesameWindowed::StateSendingReleaseWindow::Request()
    {
        communication.SesameEncodedObserver::Subject().RequestSendMessage(3);
    }

    void SesameWindowed::StateSendingReleaseWindow::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingReleaseWindow(communication.releasedWindow);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketReleaseWindow(communication.releasedWindow);
        communication.releasedWindow = 0;
    }
}
