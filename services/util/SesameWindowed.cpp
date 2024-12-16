#include "services/util/SesameWindowed.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"

namespace services
{
    SesameWindowed::SesameWindowed(SesameEncoded& delegate)
        : SesameEncodedObserver(delegate)
        , ownBufferSize(static_cast<uint16_t>(SesameEncodedObserver::Subject().MaxSendMessageSize()))
        , releaseWindowSize(static_cast<uint16_t>(SesameEncodedObserver::Subject().MessageSize(sizeof(PacketReleaseWindow))))
        , state(infra::InPlaceType<StateSendingInit>(), *this)
    {
        state->Request();
    }

    void SesameWindowed::Stop()
    {
        readerAccess.SetAction([]() {});
    }

    void SesameWindowed::RequestSendMessage(std::size_t size)
    {
        state->RequestSendMessage(size);
    }

    std::size_t SesameWindowed::MaxSendMessageSize() const
    {
        assert(initialized);
        return (std::min(ownBufferSize, maxUsableBufferSize) - sizeof(Operation) - releaseWindowSize - SesameEncodedObserver::Subject().MessageSize(sizeof(Operation))) / 2;
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
        requestedSendMessageSize = infra::none;
        state.Emplace<StateSendingInit>(*this);
        state->Request();
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
                {
                    receivedMessageReader = std::move(reader);
                    ForwardReceivedMessage(static_cast<uint16_t>(encodedSize));
                }
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

    void SesameWindowed::ForwardReceivedMessage(uint16_t encodedSize)
    {
        readerAccess.SetAction([this, encodedSize]()
            {
                releasedWindow += encodedSize;
                receivedMessageReader = nullptr;
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
            else if (requestedSendMessageSize != infra::none && SesameEncodedObserver::Subject().MessageSize(*requestedSendMessageSize + 1) + releaseWindowSize <= otherAvailableWindow)
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

        communication.requestedSendMessageSize = infra::none;
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
