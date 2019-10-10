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

        if (username.empty())
            packetHeader.connectFlags &= 0x7f;
        if (password.empty())
            packetHeader.connectFlags &= 0xbf;

        Header(PacketType::packetTypeConnect, sizeof(packetHeader) + EncodedLength(clientId) + EncodedLength(username) + EncodedLength(password));
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

        uint8_t QoS = 2;
        stream << QoS;
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizeSubscribe(const MqttClientObserver& message)
    {
        return 5 + 2 + EncodedTopicLength(message) + 1;
    }

    void MqttClientImpl::MqttFormatter::MessagePubAck(const MqttClientObserver& message)
    {
        Header(PacketType::packetTypePubAck, 2, 0);

        uint16_t packetIdentifier = 1;
        stream << infra::BigEndian<uint16_t>(packetIdentifier);
    }

    std::size_t MqttClientImpl::MqttFormatter::MessageSizePubAck(const MqttClientObserver& message)
    {
        return 4;
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
        //TODO: Should not we have a +2 here for packet identified bytes??
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
            size += sizeByte << shift;
            shift += 8;
            this->stream >> sizeByte;
        }
        size += sizeByte << shift;
    }

    MqttClientImpl::MqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, infra::Duration operationTimeout)
        : operationTimeout(operationTimeout)
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
        state->PubAck();
    }

    void MqttClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        state->SendStreamAvailable(std::move(writer));
    }

    void MqttClientImpl::DataReceived()
    {
        state->DataReceived(*ConnectionObserver::Subject().ReceiveStream());
    }

    void MqttClientImpl::Connected()
    {
        state->Connected();
    }

    void MqttClientImpl::ClosingConnection()
    {
        state->ClosingConnection();
    }

    MqttClientImpl::StateBase::StateBase(MqttClientImpl& clientConnection)
        : clientConnection(clientConnection)
    {}

    void MqttClientImpl::StateBase::Connected()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::ClosingConnection()
    {}

    void MqttClientImpl::StateBase::Publish()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::Subscribe()
    {
        std::abort();
    }

    void MqttClientImpl::StateBase::PubAck()
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

    void MqttClientImpl::StateConnecting::ClosingConnection()
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

    void MqttClientImpl::StateConnecting::DataReceived(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
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

            factory.ConnectionEstablished([this, &reader](infra::SharedPtr<MqttClientObserver> observer)
            {
                if (observer)
                {
                    observer->Attach(clientConnection);
                    clientConnection.observer = observer;
                    auto& newState = clientConnection.state.Emplace<StateConnected>(clientConnection);
                    observer->Connected();
                    newState.DataReceived(reader);
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

    void MqttClientImpl::StateConnected::ClosingConnection()
    {
        clientConnection.GetObserver().ClosingConnection();
        clientConnection.observer->Detach();
    }

    void MqttClientImpl::StateConnected::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        MqttFormatter formatter(stream);

        if (operationState == OperationState::publishing)
        {
            formatter.MessagePublish(clientConnection.GetObserver());
            writer = nullptr;
            operationTimeout.Start(clientConnection.operationTimeout, [this]() { clientConnection.Abort(); });
        }
        else if (operationState == OperationState::subscribing)
        {
            formatter.MessageSubscribe(clientConnection.GetObserver());
            writer = nullptr;
            operationTimeout.Start(clientConnection.operationTimeout, [this]() { clientConnection.Abort(); });
        }
        else
        {
            formatter.MessagePubAck(clientConnection.GetObserver());
            writer = nullptr;
        }
    }

    void MqttClientImpl::StateConnected::HandlePubAck(infra::DataInputStream& stream)
    {
        uint16_t packetIdentifier;
        stream >> packetIdentifier;

        if (stream.Failed())
            return;

        operationTimeout.Cancel();
        clientConnection.GetObserver().PublishDone();
        clientConnection.ConnectionObserver::Subject().AckReceived();
    }

    void MqttClientImpl::StateConnected::HandleSubAck(infra::DataInputStream& stream)
    {
        uint16_t packetIdentifier;
        stream >> packetIdentifier;

        uint8_t returnCode;
        stream >> returnCode;

        stream.Failed();

        operationTimeout.Cancel();
        clientConnection.GetObserver().SubscribeDone();
        clientConnection.ConnectionObserver::Subject().AckReceived();
    }

    void MqttClientImpl::StateConnected::HandlePublish(infra::DataInputStream& stream, size_t packetLength)
    {
        uint16_t topicSize;
        stream >> topicSize;
        topicSize = infra::FromBigEndian(topicSize);

        infra::BoundedString topic(receivedTopic);
        topic.resize(topicSize);
        stream >> infra::text >> topic;

        uint16_t packetIdentifier;
        stream >> packetIdentifier;
        packetIdentifier = infra::FromBigEndian(packetIdentifier);

        uint16_t payloadSize = packetLength - topicSize - 2 - 2;
        infra::BoundedString payload(receivedPayload);
        payload.resize(payloadSize);
        stream >> infra::text >> payload;

        stream.Failed();

        clientConnection.GetObserver().ReceivedNotification(topic, payload);
        clientConnection.ConnectionObserver::Subject().AckReceived();
    }

    void MqttClientImpl::StateConnected::DataReceived(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
        MqttParser parser(stream);
        if (stream.Failed())
            return;

        if (parser.GetPacketType() == PacketType::packetTypePubAck)
            HandlePubAck(stream);
        else if (parser.GetPacketType() == PacketType::packetTypeSubAck)
            HandleSubAck(stream);
        else if (parser.GetPacketType() == PacketType::packetTypePublish)
            HandlePublish(stream, parser.GetPacketSize());
    }

    void MqttClientImpl::StateConnected::Publish()
    {
        operationState = OperationState::publishing;
        clientConnection.ConnectionObserver::Subject().RequestSendStream(MqttFormatter::MessageSizePublish(clientConnection.GetObserver()));
    }

    void MqttClientImpl::StateConnected::Subscribe()
    {
        operationState = OperationState::subscribing;
        clientConnection.ConnectionObserver::Subject().RequestSendStream(MqttFormatter::MessageSizeSubscribe(clientConnection.GetObserver()));
    }

    void MqttClientImpl::StateConnected::PubAck()
    {
        operationState = OperationState::pubacking;
        clientConnection.ConnectionObserver::Subject().RequestSendStream(MqttFormatter::MessageSizePubAck(clientConnection.GetObserver()));
    }

    MqttClientConnectorImpl::MqttClientConnectorImpl(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password,
        infra::BoundedConstString address, uint16_t port, services::ConnectionFactoryWithNameResolver& connectionFactory)
        : hostname(hostname)
        , port(port)
        , connectionFactory(connectionFactory)
        , clientId(clientId)
        , username(username)
        , password(password)
    {}

    void MqttClientConnectorImpl::Connect(MqttClientObserverFactory& factory)
    {
        assert(client.Allocatable());
        clientObserverFactory = &factory;
        connectionFactory.Connect(*this);
    }

    void MqttClientConnectorImpl::CancelConnect()
    {
        connectionFactory.CancelConnect(*this);
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
        createdObserver(client.Emplace(*clientObserverFactory, clientId, username, password));
    }

    void MqttClientConnectorImpl::ConnectionFailed(ConnectFailReason reason)
    {
        switch (reason)
        {
            case ConnectFailReason::refused:
                clientObserverFactory->ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::refused);
                break;
            case ConnectFailReason::connectionAllocationFailed:
                clientObserverFactory->ConnectionFailed(MqttClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                break;
            default:
                std::abort();
        }
    }
}
