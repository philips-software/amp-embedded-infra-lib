#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/CountingOutputStream.hpp"
#include "services/network/MqttClientImpl.hpp"

namespace services
{
    MqttClientImpl::MqttFormatter::MqttFormatter(infra::DataOutputStream stream)
        : stream(stream)
    {}

    void MqttClientImpl::MqttFormatter::MessageConnect(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password)
    {
        PacketConnect packetHeader{};
        std::size_t packetSize = sizeof(packetHeader) + EncodedLength(clientId);

        if (!username.empty())
        {
            packetHeader.connectFlags |= 0x80;
            packetSize += EncodedLength(username);
        }
        if (!password.empty())
        {
            packetHeader.connectFlags |= 0x40;
            packetSize += EncodedLength(password);
        }

        Header(PacketType::packetTypeConnect, packetSize, 0);
        stream << packetHeader;
        AddString(clientId);

        if (!username.empty())
            AddString(username);
        if (!password.empty())
            AddString(password);
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizeConnect(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password)
    {
        return 5 + sizeof(PacketConnect) + EncodedLength(clientId) + EncodedLength(username) + EncodedLength(password);
    }

    void MqttClientImpl::MqttFormatter::MessagePublish(const MqttClientObserver& message)
    {
        Header(PacketType::packetTypePublish, EncodedTopicLength(message) + PayloadLength(message) + 2, 2);
        AddTopic(message);

        uint16_t packetIdentifier = 1;
        stream << infra::BigEndian<uint16_t>(packetIdentifier);
        message.FillPayload(stream.Writer());
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizePublish(const MqttClientObserver& message)
    {
        return 5 + EncodedTopicLength(message) + PayloadLength(message);
    }

    void MqttClientImpl::MqttFormatter::MessageSubscribe(const MqttClientObserver& message)
    {
        Header(PacketType::packetTypeSubscribe, EncodedTopicLength(message) + 3, 2);

        uint16_t packetIdentifier = 1;
        stream << infra::BigEndian<uint16_t>(packetIdentifier);

        AddTopic(message);

        uint8_t QoS = 1;
        stream << QoS;
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizeSubscribe(const MqttClientObserver& message)
    {
        return 5 + 2 + EncodedTopicLength(message) + 1;
    }

    void MqttClientImpl::MqttFormatter::MessagePubAck(const MqttClientObserver& message, uint16_t packetIdentifier)
    {
        Header(PacketType::packetTypePubAck, 2, 0);
        stream << infra::BigEndian<uint16_t>(packetIdentifier);
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizePubAck(const MqttClientObserver& message)
    {
        return 4;
    }

    void MqttClientImpl::MqttFormatter::MessagePing(const MqttClientObserver& message)
    {
        Header(PacketType::packetTypePingReq, 0, 0);
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizePing(const MqttClientObserver& message)
    {
        return 2;
    }

    std::size_t MqttClientImpl::MqttFormatter::EncodedLength(infra::BoundedConstString value)
    {
        return 2 + value.size();
    }

    std::size_t MqttClientImpl::MqttFormatter::EncodedTopicLength(const MqttClientObserver& message)
    {
        infra::CountingStreamWriter countingWriter;
        message.FillTopic(countingWriter);
        return 2 + countingWriter.Processed();
    }

    std::size_t MqttClientImpl::MqttFormatter::PayloadLength(const MqttClientObserver& message)
    {
        infra::CountingStreamWriter countingWriter;
        message.FillPayload(countingWriter);
        return countingWriter.Processed();
    }

    void MqttClientImpl::MqttFormatter::AddString(infra::BoundedConstString value)
    {
        stream << infra::BigEndian<uint16_t>(static_cast<uint16_t>(value.size())) << infra::StringAsByteRange(value);
    }

    void MqttClientImpl::MqttFormatter::AddTopic(const MqttClientObserver& message)
    {
        stream << infra::BigEndian<uint16_t>(static_cast<uint16_t>(EncodedTopicLength(message) - 2));
        message.FillTopic(stream.Writer());
    }

    void MqttClientImpl::MqttFormatter::Header(PacketType packetType, std::size_t size, uint8_t flags)
    {
        stream << MakePacketType(packetType, flags);

        while (size > 127)
        {
            stream << static_cast<uint8_t>((size & 0x7f) | 0x80);
            size >>= 7;
        }
        stream << static_cast<uint8_t>(size);
    }

    uint8_t MqttClientImpl::MqttFormatter::MakePacketType(PacketType packetType, uint8_t flags)
    {
        return static_cast<uint8_t>((static_cast<uint8_t>(packetType) << 4) | flags);
    }

    MqttClientImpl::MqttParser::MqttParser(infra::DataInputStream stream)
        : stream(stream)
    {
        ExtractType();
        ExtractSize();
    }

    MqttClientImpl::PacketType MqttClientImpl::MqttParser::GetPacketType() const
    {
        return packetType;
    }

    size_t MqttClientImpl::MqttParser::GetPacketSize() const
    {
        return size;
    }

    void MqttClientImpl::MqttParser::ExtractType()
    {
        uint8_t combinedPacketType;
        this->stream >> combinedPacketType;
        packetType = static_cast<PacketType>(combinedPacketType >> 4);
    }

    void MqttClientImpl::MqttParser::ExtractSize()
    {
        uint8_t sizeByte;
        uint8_t shift = 0;
        this->stream >> sizeByte;
        while (!stream.Failed() && sizeByte > 127)
        {
            size += (sizeByte & 0x7f) << shift;
            shift += 7;
            this->stream >> sizeByte;
        }
        size += sizeByte << shift;
    }

    MqttClientImpl::MqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password,
        infra::Duration operationTimeout, infra::Duration pingInterval)
        : operationTimeout(operationTimeout)
        , pingInterval(pingInterval)
        , state(infra::InPlaceType<StateConnecting>(), *this, factory, clientId, username, password)
    {}

    void MqttClientImpl::Publish()
    {
        state->Publish();
    }

    void MqttClientImpl::Subscribe()
    {
        state->Subscribe();
    }

    void MqttClientImpl::NotificationDone()
    {
        state->NotificationDone();
    }

    void MqttClientImpl::Disconnect()
    {
        Subject().AbortAndDestroy();
    }

    void MqttClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        state->SendStreamAvailable(std::move(writer));
    }

    void MqttClientImpl::DataReceived()
    {
        state->HandleDataReceived();
    }

    void MqttClientImpl::Attached()
    {
        state->Connected();
    }

    void MqttClientImpl::Detaching()
    {
        state->Detaching();
    }

    void MqttClientImpl::CancellingConnection()
    {
        state->CancellingConnection();
    }

    MqttClientImpl::StateConnected::StateConnected(MqttClientImpl& clientConnection)
        : StateBase(clientConnection)
    {
        StartPing();
    }

    infra::SharedPtr<infra::StreamWriter> MqttClientImpl::ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize)
    {
        return Observer().ReceivedNotification(topic, payloadSize);
    }

    infra::SharedPtr<infra::StreamReader> MqttClientImpl::ReceiveStream()
    {
        return ConnectionObserver::Subject().ReceiveStream();
    }

    MqttClientImpl::StateBase::StateBase(MqttClientImpl& clientConnection)
        : clientConnection(clientConnection)
    {}

    void MqttClientImpl::StateBase::Connected()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::Publish()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::Subscribe()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::NotificationDone()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::CancellingConnection()
    {
        std::abort();
    }

    MqttClientImpl::StateConnecting::StateConnecting(MqttClientImpl& clientConnection, MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password)
        : StateBase(clientConnection)
        , factory(factory)
        , clientId(clientId)
        , username(username)
        , password(password)
        , timeout(std::chrono::minutes(1), [this]() { Timeout(); })
    {}

    void MqttClientImpl::StateConnecting::Connected()
    {
        clientConnection.ConnectionObserver::Subject().RequestSendStream(MqttFormatter::MessageSizeConnect(clientId, username, password));
    }

    void MqttClientImpl::StateConnecting::Detaching()
    {
        if (!signaledFailure)
            factory.ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::initializationFailed);
    }

    void MqttClientImpl::StateConnecting::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        MqttFormatter formatter(stream);
        formatter.MessageConnect(clientId, username, password);
    }

    void MqttClientImpl::StateConnecting::HandleDataReceived()
    {
        auto reader = clientConnection.ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::softFail);

        MqttParser parser(stream);
        if (stream.Failed())
            return;

        if (parser.GetPacketType() == PacketType::packetTypeConAck)
        {
            uint8_t connectAcknowledgeFlags;
            uint8_t connectReturnCode;
            stream >> connectAcknowledgeFlags >> connectReturnCode;

            if (stream.Failed())
                return;

            clientConnection.ConnectionObserver::Subject().AckReceived();
            reader = nullptr;

            factory.ConnectionEstablished([this](infra::SharedPtr<MqttClientObserver> observer)
            {
                if (observer)
                {
                    auto& clientConnectionCopy = clientConnection;
                    auto& newState = clientConnection.state.Emplace<StateConnected>(clientConnection);
                    clientConnectionCopy.Attach(observer);
                    newState.HandleDataReceived();
                }
                else
                {
                    signaledFailure = true; // The factory already got a ConnectionEstablished, so it should not get a ConnectionFailed
                    clientConnection.Abort();
                }
            });
        }
        else
            clientConnection.Abort();
    }

    void MqttClientImpl::StateConnecting::Timeout()
    {
        signaledFailure = true;
        factory.ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::initializationTimedOut);
        clientConnection.Abort();
    }

    void MqttClientImpl::StateConnecting::CancellingConnection()
    {
        signaledFailure = true;
    }

    void MqttClientImpl::StateConnected::Detaching()
    {
        clientConnection.MqttClient::Detach();
    }

    void MqttClientImpl::StateConnected::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendOperations.front()->SendStreamAvailable(*this, *writer, clientConnection.Observer());
        sendOperations.pop_front();
        executingSend = false;

        writer = nullptr;

        ProcessSendOperations();
    }

    MqttClientImpl& MqttClientImpl::StateConnected::ClientConnection() const
    {
        return clientConnection;
    }

    void MqttClientImpl::StateConnected::HandleNotificationData(infra::DataInputStream& inputStream)
    {
        infra::DataOutputStream::WithErrorPolicy outputStream(*notificationWriter);

        while (notificationPayloadSize != 0 && !inputStream.Empty())
        {
            auto range = inputStream.ContiguousRange(notificationPayloadSize);
            outputStream << range;
            notificationPayloadSize -= static_cast<uint16_t>(range.size());
            clientConnection.ConnectionObserver::Subject().AckReceived();
        }

        if (notificationPayloadSize == 0)
            notificationWriter = nullptr;
    }

    void MqttClientImpl::StateConnected::HandlePubAck(infra::DataInputStream::WithErrorPolicy stream)
    {
        uint16_t packetIdentifier;
        stream >> packetIdentifier;

        if (stream.Failed())
            return;

        operationTimeout.Cancel();
        clientConnection.ConnectionObserver::Subject().AckReceived();
    }

    void MqttClientImpl::StateConnected::HandleSubAck(infra::DataInputStream::WithErrorPolicy stream, infra::SharedPtr<infra::StreamReader>& reader)
    {
        uint16_t packetIdentifier;
        stream >> packetIdentifier;

        uint8_t returnCode;
        stream >> returnCode;

        if (stream.Failed())
            return;

        if (infra::FromBigEndian(packetIdentifier) != 1 || returnCode == 0x80)
        {
            reader = nullptr;
            clientConnection.Abort();
            return;
        }

        operationTimeout.Cancel();
        clientConnection.ConnectionObserver::Subject().AckReceived();
    }

    void MqttClientImpl::StateConnected::HandlePublish(std::size_t packetLength, infra::DataInputStream::WithErrorPolicy stream)
    {
        uint16_t topicSize;
        stream >> topicSize;
        topicSize = infra::FromBigEndian(topicSize);

        if (stream.Failed())
            return;

        infra::BoundedString::WithStorage<128> topic;
        topic.resize(topicSize);
        stream >> infra::text >> topic;

        stream >> receivedPacketIdentifier;
        receivedPacketIdentifier = infra::FromBigEndian(receivedPacketIdentifier);

        notificationPayloadSize = static_cast<uint16_t>(packetLength) - topicSize - 2 - 2;

        if (stream.Failed())
            return;

        executingNotification = true;
        notificationWriter = clientConnection.ReceivedNotification(topic, notificationPayloadSize);
        clientConnection.ConnectionObserver::Subject().AckReceived();

        HandleNotificationData(stream);
    }

    void MqttClientImpl::StateConnected::HandleDataReceived()
    {
        auto reader = clientConnection.ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::softFail);

        while (reader != nullptr && !stream.Empty())
        {
            if (notificationPayloadSize != 0)
                HandleNotificationData(stream);

            if (executingNotification)
                return;

            MqttParser parser(stream);

            if (stream.Failed())
                return;

            if (parser.GetPacketType() == PacketType::packetTypePubAck)
                HandlePubAck(stream);
            else if (parser.GetPacketType() == PacketType::packetTypeSubAck)
                HandleSubAck(stream, reader);
            else if (parser.GetPacketType() == PacketType::packetTypePublish)
                HandlePublish(parser.GetPacketSize(), stream);
            else if (parser.GetPacketType() == PacketType::packetTypePingResp)
                HandlePingReply();
            else
            {
                reader = nullptr;
                clientConnection.Abort();
            }
        }
    }

    void MqttClientImpl::StateConnected::Publish()
    {
        QueueSendOperation<OperationPublish>();
    }

    void MqttClientImpl::StateConnected::Subscribe()
    {
        QueueSendOperation<OperationSubscribe>();
    }

    void MqttClientImpl::StateConnected::NotificationDone()
    {
        executingNotification = false;

        QueueSendOperation<OperationPubAck>();

        infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<MqttClientImpl>& client)
        {
            client->DataReceived();
        }, infra::MakeContainedSharedObject(clientConnection, clientConnection.ConnectionObserver::Subject().ObserverPtr()));
    }

    void MqttClientImpl::StateConnected::ProcessSendOperations()
    {
        if (!executingSend && !sendOperations.empty())
        {
            executingSend = true;
            clientConnection.ConnectionObserver::Subject().RequestSendStream(sendOperations.front()->MessageSize(clientConnection.Observer()));
        }
    }

    void MqttClientImpl::StateConnected::StartPing()
    {
        pingTimer.Start(clientConnection.pingInterval, [this]() { SendPing(); });
    }

    void MqttClientImpl::StateConnected::SendPing()
    {
        QueueSendOperation<OperationPing>(*this);
    }

    void MqttClientImpl::StateConnected::HandlePingReply()
    {
        clientConnection.ConnectionObserver::Subject().AckReceived();
        StartPing();
    }

    void MqttClientImpl::StateConnected::StartWaitForPingReply()
    {
        pingTimer.Start(clientConnection.operationTimeout, [this]() { PingReplyTimeout(); });
    }

    void MqttClientImpl::StateConnected::PingReplyTimeout()
    {
        clientConnection.Abort();
    }

    void MqttClientImpl::StateConnected::OperationPublish::SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer, MqttClientObserver& observer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(writer);
        MqttFormatter formatter(stream);

        connectedState.operationTimeout.Start(connectedState.ClientConnection().operationTimeout, [&connectedState]() { connectedState.ClientConnection().Abort(); });

        formatter.MessagePublish(connectedState.ClientConnection().Observer());

        observer.PublishDone();
    }

    std::size_t MqttClientImpl::StateConnected::OperationPublish::MessageSize(const MqttClientObserver& message)
    {
        return MqttFormatter::MessageSizePublish(message);
    }

    void MqttClientImpl::StateConnected::OperationSubscribe::SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer, MqttClientObserver& observer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(writer);
        MqttFormatter formatter(stream);

        connectedState.operationTimeout.Start(connectedState.ClientConnection().operationTimeout, [&connectedState]() { connectedState.ClientConnection().Abort(); });

        formatter.MessageSubscribe(connectedState.ClientConnection().Observer());

        observer.SubscribeDone();
    }

    std::size_t MqttClientImpl::StateConnected::OperationSubscribe::MessageSize(const MqttClientObserver& message)
    {
        return MqttFormatter::MessageSizeSubscribe(message);
    }

    void MqttClientImpl::StateConnected::OperationPubAck::SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer, MqttClientObserver& observer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(writer);
        MqttFormatter formatter(stream);

        formatter.MessagePubAck(connectedState.ClientConnection().Observer(), connectedState.receivedPacketIdentifier);
    }

    std::size_t MqttClientImpl::StateConnected::OperationPubAck::MessageSize(const MqttClientObserver& message)
    {
        return MqttFormatter::MessageSizePubAck(message);
    }

    MqttClientImpl::StateConnected::OperationPing::OperationPing(StateConnected& state)
        : state(state)
    {}

    void MqttClientImpl::StateConnected::OperationPing::SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer, MqttClientObserver& observer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(writer);
        MqttFormatter formatter(stream);

        formatter.MessagePing(connectedState.ClientConnection().Observer());

        state.StartWaitForPingReply();
    }

    std::size_t MqttClientImpl::StateConnected::OperationPing::MessageSize(const MqttClientObserver& message)
    {
        return MqttFormatter::MessageSizePing(message);
    }

    MqttClientConnectorImpl::MqttClientConnectorImpl(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password,
        infra::BoundedConstString hostname, uint16_t port, services::ConnectionFactoryWithNameResolver& connectionFactory)
        : hostname(hostname)
        , port(port)
        , connectionFactory(connectionFactory)
        , clientId(clientId)
        , username(username)
        , password(password)
    {}

    void MqttClientConnectorImpl::SetHostname(infra::BoundedConstString newHostname)
    {
        hostname = newHostname;
    }

    void MqttClientConnectorImpl::SetClientId(infra::BoundedConstString newClientId)
    {
        clientId = newClientId;
    }

    void MqttClientConnectorImpl::Stop(const infra::Function<void()>& onDone)
    {
        if (client.Allocatable())
            onDone();
        else
        {
            client.OnAllocatable(onDone);

            if (client)
                client->Abort();
        }
    }

    void MqttClientConnectorImpl::Connect(MqttClientObserverFactory& factory)
    {
        assert(client.Allocatable());
        connecting = true;
        clientObserverFactory = &factory;
        connectionFactory.Connect(*this);
    }

    void MqttClientConnectorImpl::CancelConnect()
    {
        if (connecting)
            connectionFactory.CancelConnect(*this);
        else
            client->CancellingConnection();
    }

    infra::BoundedConstString MqttClientConnectorImpl::Hostname() const
    {
        return hostname;
    }

    uint16_t MqttClientConnectorImpl::Port() const
    {
        return port;
    }

    void MqttClientConnectorImpl::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        connecting = false;
        createdObserver(client.Emplace(*clientObserverFactory, clientId, username, password));
    }

    void MqttClientConnectorImpl::ConnectionFailed(ConnectFailReason reason)
    {
        if (connecting)
        {
            connecting = false;
            switch (reason)
            {
                case ConnectFailReason::refused:
                    clientObserverFactory->ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::refused);
                    break;
                case ConnectFailReason::connectionAllocationFailed:
                    clientObserverFactory->ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                    break;
                case ConnectFailReason::nameLookupFailed:
                    clientObserverFactory->ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::nameLookupFailed);
                    break;
                default:
                    std::abort();
            }
        }
    }
}
