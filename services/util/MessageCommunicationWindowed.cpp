#include "services/util/MessageCommunicationWindowed.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"

namespace services
{
    MessageCommunicationWindowed::MessageCommunicationWindowed(MessageCommunicationEncoded& delegate, uint16_t ownWindowSize)
        : MessageCommunicationEncodedObserver(delegate)
        , ownWindowSize(ownWindowSize)
        , releaseWindowSize(MessageCommunicationEncodedObserver::Subject().MessageSize(sizeof(PacketReleaseWindow)))
        , state(infra::InPlaceType<StateSendingInit>(), *this)
    {
        state->Request();
    }

    void MessageCommunicationWindowed::RequestSendMessage(std::size_t size)
    {
        state->RequestSendMessage(size);
    }

    std::size_t MessageCommunicationWindowed::MaxSendMessageSize() const
    {
        return MessageCommunicationEncodedObserver::Subject().MaxSendMessageSize() - sizeof(Operation) - releaseWindowSize;
    }

    void MessageCommunicationWindowed::Initialized()
    {
        std::abort();
    }

    void MessageCommunicationWindowed::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        state->SendMessageStreamAvailable(std::move(writer));
    }

    void MessageCommunicationWindowed::MessageSent(std::size_t encodedSize)
    {
        state->MessageSent(encodedSize);
    }

    void MessageCommunicationWindowed::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        switch (stream.Extract<Operation>())
        {
            case Operation::init:
                sendInitResponse = true;
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                initialized = true;
                ReceivedInit(otherAvailableWindow);
                GetObserver().Initialized();
                break;
            case Operation::initResponse:
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                initialized = true;
                ReceivedInitResponse(otherAvailableWindow);
                GetObserver().Initialized();
                break;
            case Operation::releaseWindow:
                if (initialized)
                {
                    auto oldOtherAvailableWindow = otherAvailableWindow;
                    otherAvailableWindow += stream.Extract<infra::LittleEndian<uint16_t>>();
                    ReceivedReleaseWindow(oldOtherAvailableWindow, otherAvailableWindow);
                }
                break;
            case Operation::message:
                if (initialized)
                {
                    receivedMessageReader = std::move(reader);
                    ForwardReceivedMessage();
                }
                break;
        }

        SetNextState();
    }

    void MessageCommunicationWindowed::ForwardReceivedMessage()
    {
        releasedWindow += receivedMessageReader->Available() + 2;
        readerAccess.SetAction([this]()
            {
                receivedMessageReader = nullptr;
                SetNextState();
            });

        ForwardingReceivedMessage(*receivedMessageReader);
        GetObserver().ReceivedMessage(readerAccess.MakeShared(*receivedMessageReader));
    }

    void MessageCommunicationWindowed::SetNextState()
    {
        if (!sending && initialized)
        {
            if (sendInitResponse)
            {
                if (receivedMessageReader == nullptr)
                    state.Emplace<StateSendingInitResponse>(*this).Request();
            }
            else if (requestedSendMessageSize && MessageCommunicationEncodedObserver::Subject().MessageSize(*requestedSendMessageSize + 1) <= otherAvailableWindow - releaseWindowSize)
                state.Emplace<StateSendingMessage>(*this).Request();
            else if (releasedWindow != 0 && releaseWindowSize <= otherAvailableWindow)
                state.Emplace<StateSendingReleaseWindow>(*this).Request();
            else
                state.Emplace<StateOperational>(*this);
        }
    }

    MessageCommunicationWindowed::PacketInit::PacketInit(uint16_t window)
        : window(window)
    {}

    MessageCommunicationWindowed::PacketInitResponse::PacketInitResponse(uint16_t window)
        : window(window)
    {}

    MessageCommunicationWindowed::PacketReleaseWindow::PacketReleaseWindow(uint16_t window)
        : window(window)
    {}

    MessageCommunicationWindowed::State::State(MessageCommunicationWindowed& communication)
        : communication(communication)
    {}

    void MessageCommunicationWindowed::State::Request()
    {
        std::abort();
    }

    void MessageCommunicationWindowed::State::RequestSendMessage(std::size_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::State::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        std::abort();
    }

    void MessageCommunicationWindowed::State::MessageSent(std::size_t encodedSize)
    {
        communication.otherAvailableWindow -= encodedSize;
        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingInit::StateSendingInit(MessageCommunicationWindowed& communication)
        : State(communication)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingInit::Request()
    {
        communication.MessageCommunicationEncodedObserver::Subject().RequestSendMessage(3);
    }

    void MessageCommunicationWindowed::StateSendingInit::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInit(communication.ownWindowSize);
    }

    void MessageCommunicationWindowed::StateSendingInit::MessageSent(std::size_t encodedSize)
    {
        // Init messages do not count against window size
        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingInitResponse::StateSendingInitResponse(MessageCommunicationWindowed& communication)
        : State(communication)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::Request()
    {
        communication.MessageCommunicationEncodedObserver::Subject().RequestSendMessage(3);
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInitResponse(communication.ownWindowSize);

        communication.releasedWindow = 0;
        communication.sendInitResponse = false;
    }

    MessageCommunicationWindowed::StateOperational::StateOperational(MessageCommunicationWindowed& communication)
        : State(communication)
    {}

    void MessageCommunicationWindowed::StateOperational::RequestSendMessage(std::size_t size)
    {
        communication.requestedSendMessageSize = size;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingMessage::StateSendingMessage(MessageCommunicationWindowed& communication)
        : State(communication)
        , requestedSize(*communication.requestedSendMessageSize + 1)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingMessage::Request()
    {
        communication.MessageCommunicationEncodedObserver::Subject().RequestSendMessage(requestedSize);
    }

    void MessageCommunicationWindowed::StateSendingMessage::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << Operation::message;

        messageWriter = std::move(writer);
        communication.requestedSendMessageSize = infra::none;
        communication.GetObserver().SendMessageStreamAvailable(writerAccess.MakeShared(*messageWriter));
    }

    void MessageCommunicationWindowed::StateSendingMessage::MessageSent(std::size_t encodedSize)
    {
        communication.otherAvailableWindow -= encodedSize;
    }

    void MessageCommunicationWindowed::StateSendingMessage::OnSent()
    {
        messageWriter = nullptr;
        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingReleaseWindow::StateSendingReleaseWindow(MessageCommunicationWindowed& communication)
        : State(communication)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::Request()
    {
        communication.MessageCommunicationEncodedObserver::Subject().RequestSendMessage(3);
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketReleaseWindow(communication.releasedWindow);
        communication.releasedWindow = 0;
    }
}
