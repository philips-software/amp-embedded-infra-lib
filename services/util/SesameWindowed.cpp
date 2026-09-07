#include "services/util/SesameWindowed.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"
#include <algorithm>

namespace services
{
    namespace
    {
        // This reader is used to put the Operation::message byte ('\x4') in front of a message, in order to exactly calculate the encoded size
        // Only ExtractContiguousRange(), Empty(), and Available() are used by SesameCobs, so only those methods are implemented here.
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
                    return infra::Head(infra::Head(infra::MakeByteRange(character), std::exchange(extraCharacter, 0)), max);
                else
                    return infra::LimitedStreamReader::ExtractContiguousRange(max);
            }

            infra::ConstByteRange PeekContiguousRange(std::size_t start) override
            {
                std::abort();
            }

            bool Empty() const override
            {
                return extraCharacter == 0 && infra::LimitedStreamReader::Empty();
            }

            std::size_t Available() const override
            {
                return infra::LimitedStreamReader::Available() + extraCharacter;
            }

        private:
            static const char character;
            std::size_t extraCharacter = 1;
        };

        const char ExtraCharacterReader::character = '\x4';

        uint8_t ToMessageOperation(SesameChannel channel)
        {
            switch (channel)
            {
                case SesameChannel::red:
                    return 4;
                case SesameChannel::blue:
                    return 5;
            }

            std::abort();
        }

        SesameChannel ToChannel(uint8_t operation)
        {
            switch (operation)
            {
                case 4:
                    return SesameChannel::red;
                case 5:
                    return SesameChannel::blue;
                default:
                    std::abort();
            }
        }

    }

    SesameWindowed::SesameWindowed(infra::BoundedDeque<uint8_t>& redReceivedMessage, infra::BoundedDeque<uint8_t>& blueReceivedMessage, uint8_t splitBuffers, SesameEncoded& delegate, SesameInitializer& sesameInitializer)
        : SesameEncodedObserver(delegate)
        , redChannelAdministration(redReceivedMessage)
        , blueChannelAdministration(blueReceivedMessage)
        , splitBuffers(splitBuffers)
        , sesameInitializer(sesameInitializer)
        , ownBufferSize(static_cast<uint16_t>(SesameEncodedObserver::Subject().MaxSendMessageSize()))
        , releaseWindowSize(static_cast<uint16_t>(SesameEncodedObserver::Subject().WorstCaseEncodedMessageSize(sizeof(PacketReleaseWindow))))
        , state(std::in_place_type_t<StateSendingInit>(), *this)
    {
        state->Request();
    }

    void SesameWindowed::RequestSendMessage(std::size_t size, SesameChannel channel)
    {
        assert(size <= MaxSendMessageSize());
        state->RequestSendMessage(size, channel);
    }

    SesameWindowed::ChannelAdministration::ChannelAdministration(infra::BoundedDeque<uint8_t>& receivedMessage)
        : receivedMessage(receivedMessage)
    {}

    void SesameWindowed::ChannelAdministration::Reset()
    {
        currentReceiveMessageSize = 0;
        currentReceiveMessageReader = std::nullopt;
        readerAccess.SetAction(infra::emptyFunction);
        releasedWindow = 0;
        requestedSendMessageSize = std::nullopt;
        receivedMessage.clear();
    }

    void SesameWindowed::ChannelAdministration::ResetReading()
    {
        readerAccess.SetAction([this]()
            {
                currentReceiveMessageReader = std::nullopt;
                currentReceiveMessageSize = 0;
                receivedMessage.clear();
            });
    }

    bool SesameWindowed::ChannelAdministration::Receiving() const
    {
        return currentReceiveMessageReader != std::nullopt || readerAccess.Referenced();
    }

    bool SesameWindowed::ChannelAdministration::HasSavedMessages() const
    {
        return !receivedMessage.empty();
    }

    std::size_t SesameWindowed::MaxSendMessageSize() const
    {
        assert(initialized);
        return SesameEncodedObserver::Subject().WorstCaseDecodedMessageSize((maxUsableBufferSize - releaseWindowSize) / splitBuffers) - sizeof(Operation);
    }

    void SesameWindowed::Reset()
    {
        SesameEncodedObserver::Subject().Reset();
        assert(!redChannelAdministration.Receiving());
        assert(!blueChannelAdministration.Receiving());
        initialized = false;
        requestingInitialization = false;
        sentInitResponse = false;
        otherAvailableWindow = 0;
        maxUsableBufferSize = 0;
        controlReleasedWindow = 0;
        sendInitResponse = false;
        sending = false;
        requestedSendMessageChannel = SesameChannel::red;
        redChannelAdministration.Reset();
        blueChannelAdministration.Reset();
        // Now wait for an init message to be received; use state Operational for this
        state.Emplace<StateOperational>(*this);
    }

    void SesameWindowed::ResetReading()
    {
        redChannelAdministration.ResetReading();
        blueChannelAdministration.ResetReading();
    }

    void SesameWindowed::ReceivedInit(uint16_t newWindow)
    {}

    void SesameWindowed::ReceivedInitResponse(uint16_t newWindow)
    {}

    void SesameWindowed::ReceivedReleaseWindow(uint16_t oldWindow, uint16_t newWindow)
    {}

    void SesameWindowed::ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader)
    {}

    void SesameWindowed::SendingInit(uint16_t newWindow)
    {}

    void SesameWindowed::SendingInitResponse(uint16_t newWindow)
    {}

    void SesameWindowed::SendingReleaseWindow(uint16_t deltaWindow)
    {}

    void SesameWindowed::SendingMessage([[maybe_unused]] infra::StreamWriter& writer, [[maybe_unused]] SesameChannel channel)
    {}

    void SesameWindowed::SettingOperational(std::optional<std::size_t> requestedSize, uint16_t releasedWindow, uint16_t otherWindow)
    {}

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

    void SesameWindowed::ReceivedMessage(infra::StreamReaderWithRewinding& reader, std::size_t encodedSize)
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);
        auto operation = stream.Extract<Operation>();
        switch (operation)
        {
            case Operation::init:
            {
                auto window = stream.Extract<infra::LittleEndian<uint16_t>>();
                sesameInitializer.InitInformationReceived(reader);
                ReceivedInit(window);
                requestingInitialization = true;
                sesameInitializer.InitializationRequested([this, window]()
                    {
                        otherAvailableWindow = window;
                        sendInitResponse = true;
                        requestingInitialization = false;
                        ReceivedInitialize();
                        SetNextState();
                    });
                break;
            }
            case Operation::initResponse:
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                ReceivedInitResponse(otherAvailableWindow);
                controlReleasedWindow = static_cast<uint16_t>(encodedSize);
                sesameInitializer.InitInformationReceived(reader);
                // When peers send an init message at the same time, both will respond with an init response.
                // In that case, the first init response received will already trigger ReceivedInitialize()
                if (!sentInitResponse && !requestingInitialization)
                    ReceivedInitialize();
                break;
            case Operation::releaseWindow:
                if (initialized)
                {
                    controlReleasedWindow += encodedSize;
                    auto oldOtherAvailableWindow = otherAvailableWindow;
                    otherAvailableWindow += stream.Extract<infra::LittleEndian<uint16_t>>();
                    ReceivedReleaseWindow(oldOtherAvailableWindow, otherAvailableWindow);
                }
                break;
            case Operation::messageRed:
            case Operation::messageBlue:
                if (initialized)
                {
                    auto channel = ToChannel(static_cast<uint8_t>(operation));
                    auto& channelAdministration = ChannelAdministrationFor(channel);
                    SaveReceivedMessage(reader, channelAdministration);
                    TryForwardReceivedMessage(channelAdministration, channel);
                }
                break;
        }

        SetNextState();
    }

    void SesameWindowed::ReceivedInitialize()
    {
        maxUsableBufferSize = std::min<uint16_t>(SesameEncodedObserver::Subject().MaxSendMessageSize(), otherAvailableWindow);
        initialized = true;
        GetObserver().Initialized();
    }

    SesameWindowed::ChannelAdministration& SesameWindowed::ChannelAdministrationFor(SesameChannel channel)
    {
        if (channel == SesameChannel::red)
            return redChannelAdministration;
        else
            return blueChannelAdministration;
    }

    const SesameWindowed::ChannelAdministration& SesameWindowed::ChannelAdministrationFor(SesameChannel channel) const
    {
        if (channel == SesameChannel::red)
            return redChannelAdministration;
        else
            return blueChannelAdministration;
    }

    uint16_t SesameWindowed::ReleasedWindow() const
    {
        return controlReleasedWindow + redChannelAdministration.releasedWindow + blueChannelAdministration.releasedWindow;
    }

    void SesameWindowed::SaveReceivedMessage(infra::StreamReader& reader, ChannelAdministration& channelAdministration)
    {
        infra::BoundedDequeOutputStream stream(channelAdministration.receivedMessage);

        stream << static_cast<uint16_t>(reader.Available());
        while (!reader.Empty())
            stream << reader.ExtractContiguousRange(std::numeric_limits<uint16_t>::max());
    }

    void SesameWindowed::TryForwardReceivedMessage(ChannelAdministration& channelAdministration, SesameChannel channel)
    {
        if (channelAdministration.currentReceiveMessageReader == std::nullopt && channelAdministration.HasSavedMessages())
        {
            infra::BoundedDequeInputStream stream(channelAdministration.receivedMessage);
            channelAdministration.currentReceiveMessageSize = stream.Extract<uint16_t>();
            auto encodedSize = SesameEncodedObserver::Subject().MessageSize(ExtraCharacterReader(stream.Reader(), channelAdministration.currentReceiveMessageSize));
            channelAdministration.receivedMessage.erase(channelAdministration.receivedMessage.begin(), channelAdministration.receivedMessage.begin() + 2);

            channelAdministration.currentReceiveMessageReader.emplace(std::in_place, channelAdministration.receivedMessage, channelAdministration.currentReceiveMessageSize);
            ForwardReceivedMessage(channelAdministration, channel, static_cast<uint16_t>(encodedSize));
        }
    }

    void SesameWindowed::ForwardReceivedMessage(ChannelAdministration& channelAdministration, SesameChannel channel, uint16_t encodedSize)
    {
        channelAdministration.readerAccess.SetAction([this, channel, encodedSize]()
            {
                auto& channelAdministration = ChannelAdministrationFor(channel);
                channelAdministration.releasedWindow += encodedSize;
                channelAdministration.currentReceiveMessageReader = std::nullopt;
                channelAdministration.receivedMessage.erase(channelAdministration.receivedMessage.begin(), channelAdministration.receivedMessage.begin() + channelAdministration.currentReceiveMessageSize);
                TryForwardReceivedMessage(channelAdministration, channel);
                SetNextState();
            });

        ForwardingReceivedMessage(*channelAdministration.currentReceiveMessageReader);
        GetObserver().ReceivedMessage(channelAdministration.readerAccess.MakeShared(*channelAdministration.currentReceiveMessageReader), channel);
    }

    bool SesameWindowed::HasReceivingChannels() const
    {
        return redChannelAdministration.Receiving() || blueChannelAdministration.Receiving();
    }

    std::optional<SesameChannel> SesameWindowed::RequestedSendMessageChannel() const
    {
        if (RequestedSendMessageSize(requestedSendMessageChannel) != std::nullopt)
            return requestedSendMessageChannel;

        auto otherChannel = requestedSendMessageChannel == SesameChannel::red ? SesameChannel::blue : SesameChannel::red;
        if (RequestedSendMessageSize(otherChannel) != std::nullopt)
            return otherChannel;

        return std::nullopt;
    }

    std::optional<std::size_t> SesameWindowed::RequestedSendMessageSize(SesameChannel channel) const
    {
        return ChannelAdministrationFor(channel).requestedSendMessageSize;
    }

    void SesameWindowed::ResetRequestedSendMessage(SesameChannel channel)
    {
        ChannelAdministrationFor(channel).requestedSendMessageSize = std::nullopt;
    }

    void SesameWindowed::SetNextState()
    {
        if (!sending && initialized)
        {
            auto requestedChannel = RequestedSendMessageChannel();
            if (sendInitResponse)
            {
                if (!HasReceivingChannels())
                    state.Emplace<StateSendingInitResponse>(*this).Request();
            }
            else if (requestedChannel != std::nullopt
                     && SesameEncodedObserver::Subject().WorstCaseEncodedMessageSize(*RequestedSendMessageSize(*requestedChannel) + 1) + releaseWindowSize <= otherAvailableWindow)
                state.Emplace<StateSendingMessage>(*this).Request();
            else if (ReleasedWindow() >= (ownBufferSize - releaseWindowSize) / splitBuffers && releaseWindowSize <= otherAvailableWindow)
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

    void SesameWindowed::State::RequestSendMessage(std::size_t size, SesameChannel channel)
    {
        communication.ChannelAdministrationFor(channel).requestedSendMessageSize = size;
        communication.requestedSendMessageChannel = channel;
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
        communication.SesameEncodedObserver::Subject().RequestSendMessage(sizeof(PacketInit) + communication.sesameInitializer.InitInformation().size());
    }

    void SesameWindowed::StateSendingInit::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingInit(communication.ownBufferSize);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInit(communication.ownBufferSize) << communication.sesameInitializer.InitInformation();
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
        communication.sentInitResponse = true;
    }

    void SesameWindowed::StateSendingInitResponse::Request()
    {
        communication.SesameEncodedObserver::Subject().RequestSendMessage(sizeof(PacketInitResponse) + communication.sesameInitializer.InitInformation().size());
    }

    void SesameWindowed::StateSendingInitResponse::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingInitResponse(communication.ownBufferSize);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInitResponse(communication.ownBufferSize) << communication.sesameInitializer.InitInformation();

        communication.controlReleasedWindow = 0;
        communication.redChannelAdministration.releasedWindow = 0;
        communication.blueChannelAdministration.releasedWindow = 0;
        communication.sendInitResponse = false;
    }

    SesameWindowed::StateOperational::StateOperational(SesameWindowed& communication)
        : State(communication)
    {
        communication.SettingOperational(communication.RequestedSendMessageSize(communication.requestedSendMessageChannel), communication.ReleasedWindow(), communication.otherAvailableWindow);
    }

    void SesameWindowed::StateOperational::RequestSendMessage(std::size_t size, SesameChannel channel)
    {
        communication.ChannelAdministrationFor(channel).requestedSendMessageSize = size;
        communication.requestedSendMessageChannel = channel;
        communication.SetNextState();
    }

    SesameWindowed::StateSendingMessage::StateSendingMessage(SesameWindowed& communication)
        : State(communication)
        , channel(*communication.RequestedSendMessageChannel())
        , requestedSize(*communication.RequestedSendMessageSize(channel) + 1)
    {
        communication.sending = true;
    }

    void SesameWindowed::StateSendingMessage::Request()
    {
        communication.SesameEncodedObserver::Subject().RequestSendMessage(requestedSize);
    }

    void SesameWindowed::StateSendingMessage::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingMessage(*writer, channel);
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << static_cast<Operation>(ToMessageOperation(channel));

        communication.ResetRequestedSendMessage(channel);
        communication.GetObserver().SendMessageStreamAvailable(std::move(writer), channel);
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
        communication.SesameEncodedObserver::Subject().RequestSendMessage(sizeof(PacketReleaseWindow));
    }

    void SesameWindowed::StateSendingReleaseWindow::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        communication.SendingReleaseWindow(communication.ReleasedWindow());
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketReleaseWindow(communication.ReleasedWindow());
        communication.controlReleasedWindow = 0;
        communication.redChannelAdministration.releasedWindow = 0;
        communication.blueChannelAdministration.releasedWindow = 0;
    }
}
