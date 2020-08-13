#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"
#include "infra/util/PostAssign.hpp"
#include "services/network/MessageCommunication.hpp"

namespace services
{
    WindowedMessageCommunication::WindowedMessageCommunication(infra::BoundedDeque<uint8_t>& receivedData, MessageCommunicationReceiveOnInterrupt& messageCommunication)
        : MessageCommunicationReceiveOnInterruptObserver(messageCommunication)
        , receivedData(receivedData)
        , state(infra::InPlaceType<StateSendingInit>(), *this)
    {}

    void WindowedMessageCommunication::RequestSendMessage(uint16_t size)
    {
        state->RequestSendMessage(size);
    }

    std::size_t WindowedMessageCommunication::MaxSendMessageSize() const
    {
        return MessageCommunicationReceiveOnInterruptObserver::Subject().MaxSendMessageSize() - sizeof(Operation);
    }

    void WindowedMessageCommunication::ReceivedMessageOnInterrupt(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);
        switch (stream.Extract<Operation>())
        {
            case Operation::init:
                state->ReceivedInit(stream.Extract<infra::LittleEndian<uint16_t>>());
                break;
            case Operation::initResponse:
                state->ReceivedInitResponse(stream.Extract<infra::LittleEndian<uint16_t>>());
                break;
            case Operation::releaseWindow:
                state->ReceivedReleaseWindow(stream.Extract<infra::LittleEndian<uint16_t>>());
                break;
            case Operation::message:
                state->ReceivedMessage(reader);
                break;
        }
    }

    void WindowedMessageCommunication::EvaluateReceiveMessage()
    {
        if (!notificationScheduled && reader.Allocatable())
        {
            infra::BoundedDequeInputStream stream(receivedData, infra::softFail);
            auto size = stream.Extract<uint16_t>();
            if (!stream.Failed() && stream.Available() <= size)
            {
                if (!notificationScheduled.exchange(true))
                    infra::EventDispatcher::Instance().Schedule([this]()
                    {
                        infra::BoundedDequeInputStream stream(receivedData);
                        auto size = stream.Extract<uint16_t>();

                        reader.OnAllocatable([this, size]() { receivedData.erase(receivedData.begin(), receivedData.begin() + size); EvaluateReceiveMessage(); });
                        receivedData.erase(receivedData.begin(), receivedData.begin() + 2);
                        GetObserver().ReceivedMessage(reader.Emplace(infra::inPlace, receivedData, size));

                        notificationScheduled = false;
                        EvaluateReceiveMessage();
                    });
            }
        }
    }

    void WindowedMessageCommunication::SetNextState(bool sendInitResponse)
    {
        if (sendInitResponse)
            state.Emplace<StateSendingInitResponse>(*this);
        else if (requestedSendMessageSize && WindowSize(*requestedSendMessageSize) <= otherAvailableWindow)
            state.Emplace<StateSendingMessage>(*this);
        else
            state.Emplace<StateOperational>(*this);
    }

    uint16_t WindowedMessageCommunication::WindowSize(uint16_t messageSize) const
    {
        return messageSize + sizeof(uint16_t);
    }

    uint16_t WindowedMessageCommunication::AvailableWindow() const
    {
        return static_cast<uint16_t>(receivedData.max_size() - receivedData.size());
    }

    WindowedMessageCommunication::PacketInit::PacketInit(uint16_t window)
        : window(window)
    {}

    WindowedMessageCommunication::PacketInitResponse::PacketInitResponse(uint16_t window)
        : window(window)
    {}

    WindowedMessageCommunication::StateSendingInit::StateSendingInit(WindowedMessageCommunication& communication)
        : communication(communication)
    {
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(3, [this](uint16_t) { OnSent(); });
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInit(communication.AvailableWindow());
    }

    void WindowedMessageCommunication::StateSendingInit::OnSent()
    {
        communication.SetNextState(sendInitResponse);
    }

    void WindowedMessageCommunication::StateSendingInit::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void WindowedMessageCommunication::StateSendingInit::ReceivedInit(uint16_t availableWindow)
    {
        sendInitResponse = true;
        communication.otherAvailableWindow = availableWindow;
    }

    void WindowedMessageCommunication::StateSendingInit::ReceivedInitResponse(uint16_t availableWindow)
    {}  // A response to our init cannot be received before our init is completely sent. Therefore, ignore it.

    void WindowedMessageCommunication::StateSendingInit::ReceivedReleaseWindow(uint16_t additionalAvailableWindow)
    {}  // Ignore messages received before completing initial handshake.

    void WindowedMessageCommunication::StateSendingInit::ReceivedMessage(infra::StreamReader& reader)
    {}  // Ignore messages received before completing initial handshake.

    WindowedMessageCommunication::StateReceivable::StateReceivable(WindowedMessageCommunication& communication)
        : communication(communication)
    {}

    void WindowedMessageCommunication::StateReceivable::ReceivedMessage(infra::StreamReader& reader)
    {
        uint16_t size = static_cast<uint16_t>(reader.Available());

        infra::DataInputStream::WithErrorPolicy from(reader, infra::noFail);
        infra::BoundedDequeOutputStream to(communication.receivedData, infra::noFail);

        to << size;

        while (size != 0)
        {
            auto range = from.ContiguousRange(size);
            to << range;
            size -= static_cast<uint16_t>(range.size());
        }

        communication.EvaluateReceiveMessage();
    }

    void WindowedMessageCommunication::StateReceivable::NextState()
    {
        communication.SetNextState(sendInitResponse);
    }

    WindowedMessageCommunication::StateSendingInitResponse::StateSendingInitResponse(WindowedMessageCommunication& communication)
        : StateReceivable(communication)
    {
        assert(communication.receivedData.empty());
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(3, [this](uint16_t) { OnSent(); });
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInitResponse(communication.AvailableWindow());
    }

    void WindowedMessageCommunication::StateSendingInitResponse::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void WindowedMessageCommunication::StateSendingInitResponse::ReceivedInit(uint16_t availableWindow)
    {
        communication.otherAvailableWindow = availableWindow;
        sendInitResponse = true;
    }

    void WindowedMessageCommunication::StateSendingInitResponse::ReceivedInitResponse(uint16_t availableWindow)
    {
        communication.otherAvailableWindow = availableWindow;
    }

    void WindowedMessageCommunication::StateSendingInitResponse::ReceivedReleaseWindow(uint16_t additionalAvailableWindow)
    {}  // Init response has not yet been sent, so we have not yet sent any data, so there is nothing to release

    void WindowedMessageCommunication::StateSendingInitResponse::OnSent()
    {
        communication.SetNextState(sendInitResponse);
    }

    void WindowedMessageCommunication::StateOperational::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
        NextState();
    }

    void WindowedMessageCommunication::StateOperational::ReceivedInit(uint16_t availableWindow)
    {
        communication.otherAvailableWindow = availableWindow;
        sendInitResponse = true;
        NextState();
    }

    void WindowedMessageCommunication::StateOperational::ReceivedInitResponse(uint16_t availableWindow)
    {
        communication.otherAvailableWindow = availableWindow;
        NextState();
    }

    void WindowedMessageCommunication::StateOperational::ReceivedReleaseWindow(uint16_t additionalAvailableWindow)
    {
        communication.otherAvailableWindow += additionalAvailableWindow;
        NextState();
    }

    WindowedMessageCommunication::StateSendingMessage::StateSendingMessage(WindowedMessageCommunication& communication)
        : StateReceivable(communication)
    {
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(*communication.requestedSendMessageSize + 1, [this](uint16_t sent) { OnSent(sent); });

        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << Operation::message;

        communication.GetObserver().SendMessageStreamAvailable(std::move(writer));
        communication.requestedSendMessageSize = infra::none;
    }

    void WindowedMessageCommunication::StateSendingMessage::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void WindowedMessageCommunication::StateSendingMessage::ReceivedInit(uint16_t availableWindow)
    {
        sendInitResponse = true;
    }

    void WindowedMessageCommunication::StateSendingMessage::ReceivedInitResponse(uint16_t availableWindow)
    {
        communication.otherAvailableWindow = availableWindow;
    }

    void WindowedMessageCommunication::StateSendingMessage::ReceivedReleaseWindow(uint16_t additionalAvailableWindow)
    {
        communication.otherAvailableWindow += additionalAvailableWindow;
    }

    void WindowedMessageCommunication::StateSendingMessage::OnSent(uint16_t sent)
    {
        communication.otherAvailableWindow -= communication.WindowSize(sent);
        NextState();
    }
}
