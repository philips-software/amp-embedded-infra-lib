#include "services/util/MessageCommunicationWindowed.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"
#include "infra/util/PostAssign.hpp"

namespace services
{
    MessageCommunicationWindowed::MessageCommunicationWindowed(MessageCommunication& delegate, uint16_t ownWindowSize)
        : MessageCommunicationObserver(delegate)
        , ownWindowSize(ownWindowSize)
        , state(infra::InPlaceType<StateSendingInit>(), *this)
    {
        state->Request();
    }

    void MessageCommunicationWindowed::RequestSendMessage(uint16_t size)
    {
        state->RequestSendMessage(size);
    }

    std::size_t MessageCommunicationWindowed::MaxSendMessageSize() const
    {
        return MessageCommunicationObserver::Subject().MaxSendMessageSize() - sizeof(Operation);
    }

    void MessageCommunicationWindowed::Initialized()
    {
        std::abort();
    }

    void MessageCommunicationWindowed::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        state->SendMessageStreamAvailable(std::move(writer));
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
                GetObserver().Initialized();
                break;
            case Operation::initResponse:
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                initialized = true;
                GetObserver().Initialized();
                break;
            case Operation::releaseWindow:
                if (initialized)
                    otherAvailableWindow += stream.Extract<infra::LittleEndian<uint16_t>>();
                break;
            case Operation::message:
                if (initialized)
                {
                    this->reader = std::move(reader);
                    EvaluateReceiveMessage();
                }
                break;
        }

        SetNextState();
    }

    void MessageCommunicationWindowed::EvaluateReceiveMessage()
    {
        releasedWindow += reader->Available() + 2;
        readerAccess.SetAction([this]()
            {
                reader = nullptr;
                SetNextState();
            });

        GetObserver().ReceivedMessage(readerAccess.MakeShared(*reader));
    }

    void MessageCommunicationWindowed::SetNextState()
    {
        if (!sending)
        {
            if (sendInitResponse)
            {
                if (reader == nullptr)
                    state.Emplace<StateSendingInitResponse>(*this).Request();
            }
            else if (requestedSendMessageSize && WindowSize(*requestedSendMessageSize) <= otherAvailableWindow)
                state.Emplace<StateSendingMessage>(*this).Request();
            else if (releasedWindow != 0)
                state.Emplace<StateSendingReleaseWindow>(*this).Request();
            else
                state.Emplace<StateOperational>(*this);
        }
    }

    uint16_t MessageCommunicationWindowed::WindowSize(uint16_t messageSize) const
    {
        return messageSize + sizeof(uint16_t);
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

    void MessageCommunicationWindowed::State::Request()
    {
        std::abort();
    }

    void MessageCommunicationWindowed::State::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        std::abort();
    }

    MessageCommunicationWindowed::StateSendingInit::StateSendingInit(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingInit::Request()
    {
        communication.MessageCommunicationObserver::Subject().RequestSendMessage(3);
    }

    void MessageCommunicationWindowed::StateSendingInit::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingInit::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInit(communication.ownWindowSize);
        writer = nullptr;

        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingInitResponse::StateSendingInitResponse(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::Request()
    {
        communication.MessageCommunicationObserver::Subject().RequestSendMessage(3);
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInitResponse(communication.ownWindowSize);

        communication.releasedWindow = 0;
        communication.sendInitResponse = false;
        writer = nullptr;

        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateOperational::StateOperational(MessageCommunicationWindowed& communication)
        : communication(communication)
    {}

    void MessageCommunicationWindowed::StateOperational::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingMessage::StateSendingMessage(MessageCommunicationWindowed& communication)
        : communication(communication)
        , requestedSize(*communication.requestedSendMessageSize + 1)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingMessage::Request()
    {
        communication.MessageCommunicationObserver::Subject().RequestSendMessage(requestedSize);
    }

    void MessageCommunicationWindowed::StateSendingMessage::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingMessage::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << Operation::message;

        this->writer = std::move(writer);
        communication.requestedSendMessageSize = infra::none;
        communication.GetObserver().SendMessageStreamAvailable(writerAccess.MakeShared(*this->writer));
    }

    void MessageCommunicationWindowed::StateSendingMessage::OnSent()
    {
        writer = nullptr;
        communication.otherAvailableWindow -= communication.WindowSize(requestedSize - 1);
        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingReleaseWindow::StateSendingReleaseWindow(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        communication.sending = true;
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::Request()
    {
        communication.MessageCommunicationObserver::Subject().RequestSendMessage(3);
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketReleaseWindow(communication.releasedWindow);
        communication.releasedWindow = 0;
        writer = nullptr;

        communication.sending = false;
        communication.SetNextState();
    }
}
