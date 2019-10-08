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
        Header(PacketType::packetTypeConnect, sizeof(packetHeader) + EncodedLength(clientId) + EncodedLength(username) + EncodedLength(password));
        stream << packetHeader;
        AddString(clientId);
        AddString(username);
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

    MqttClientImpl::MqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, infra::Duration publishTimeout)
        : publishTimeout(publishTimeout)
        , state(infra::InPlaceType<StateConnecting>(), *this, factory, clientId, username, password)
    {}

    void MqttClientImpl::Publish()
    {
        state->Publish();
    }

    void MqttClientImpl::Subscribe()
    {}

    void MqttClientImpl::NotificationDone()
    {}

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
        formatter.MessagePublish(clientConnection.GetObserver());

        writer = nullptr;
        publishTimeout.Start(clientConnection.publishTimeout, [this]() { clientConnection.Abort(); });
    }

    void MqttClientImpl::StateConnected::DataReceived(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
        MqttParser parser(stream);
        if (stream.Failed())
            return;

        if (parser.GetPacketType() == PacketType::packetTypePubAck)
        {
            uint16_t packetIdentifier;
            stream >> packetIdentifier;

            if (stream.Failed())
                return;

            publishTimeout.Cancel();
            clientConnection.GetObserver().PublishDone();
            clientConnection.ConnectionObserver::Subject().AckReceived();
        }
    }

    void MqttClientImpl::StateConnected::Publish()
    {
        clientConnection.ConnectionObserver::Subject().RequestSendStream(MqttFormatter::MessageSizePublish(clientConnection.GetObserver()));
    }

    MqttClientConnectorImpl::MqttClientConnectorImpl(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, services::IPv4Address address, uint16_t port, services::ConnectionFactory& connectionFactory)
        : address(address)
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

    IPAddress MqttClientConnectorImpl::Address() const
    {
        return address;
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
