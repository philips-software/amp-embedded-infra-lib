#ifndef SERVICES_MQTT_IMPL_HPP
#define SERVICES_MQTT_IMPL_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/Mqtt.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace services
{
    class MqttClientImpl
        : public ConnectionObserver
        , public MqttClient
    {
    public:
        MqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, infra::Duration operationTimeout = std::chrono::seconds(30));

        // Implementation of MqttClient
        virtual void Publish() override;
        virtual void Subscribe() override;
        virtual void NotificationDone() override;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;

    protected:
        virtual void ReceivedNotification(infra::BoundedConstString topic, infra::BoundedConstString payload);

    private:
        enum class PacketType : uint8_t
        {
            packetTypeConnect = 1,
            packetTypeConAck = 2,
            packetTypePublish = 3,
            packetTypePubAck = 4,
            packetTypePubRec = 5,
            packetTypePubRel = 6,
            packetTypePubComp = 7,
            packetTypeSubscribe = 8,
            packetTypeSubAck = 9,
            packetTypeUnsubscribe = 10,
            packetTypeUnsubAck = 11,
            packetTypePingReq = 12,
            packetTypePingResp = 13,
            packetTypeDisconnect = 14
        };

        class MqttFormatter
        {
        private:
            template<PacketType packetType>
                struct InPlaceType {};

        public:
            MqttFormatter(infra::DataOutputStream stream);

            void MessageConnect(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password);
            static std::size_t MessageSizeConnect(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password);
            void MessagePublish(const MqttClientObserver& message);
            static std::size_t MessageSizePublish(const MqttClientObserver& message);
            void MessageSubscribe(const MqttClientObserver& message);
            static std::size_t MessageSizeSubscribe(const MqttClientObserver& message);
            void MessagePubAck(const MqttClientObserver& message, uint16_t packetIdentifier);
            static std::size_t MessageSizePubAck(const MqttClientObserver& message);

        private:
            static std::size_t EncodedLength(infra::BoundedConstString value);
            static std::size_t EncodedTopicLength(const MqttClientObserver& message);
            static std::size_t PayloadLength(const MqttClientObserver& message);
            void AddString(infra::BoundedConstString value);
            void AddTopic(const MqttClientObserver& message);
            void Header(PacketType packetType, std::size_t size, uint8_t flags = 0);
            uint8_t MakePacketType(PacketType packetType, uint8_t flags);

        private:
            infra::DataOutputStream stream;
        };

        class MqttParser
        {
        public:
            MqttParser(infra::DataInputStream stream);

            PacketType GetPacketType() const;
            size_t GetPacketSize() const;

        private:
            void ExtractType();
            void ExtractSize();

        private:
            infra::DataInputStream stream;
            PacketType packetType;
            uint32_t size = 0;
        };

        struct PacketConnect
        {
            infra::BigEndian<uint16_t> protocolNameLength = 4;
            std::array<char, 4> protocolName{{ 'M', 'Q', 'T', 'T' }};
            uint8_t protocolLevel = 4;
            uint8_t connectFlags = 0x02;    // clean session, no will
            infra::BigEndian<uint16_t> keepAlive = 0;
        };

        class StateBase
        {
        public:
            StateBase(MqttClientImpl& clientConnection);
            StateBase(const StateBase& other) = delete;
            StateBase& operator=(const StateBase& other) = delete;
            virtual ~StateBase() = default;

        public:
            virtual void Connected();
            virtual void ClosingConnection() = 0;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
            virtual void HandleDataReceived() = 0;

            virtual void Publish();
            virtual void Subscribe();
            virtual void NotificationDone();

        protected:
            infra::StreamReader& NewReceiveStream();
            MqttClientImpl& clientConnection;

        private:
            infra::SharedPtr<infra::StreamReader> receiveStream;
        };

        class StateConnecting
            : public StateBase
        {
        public:
            StateConnecting(MqttClientImpl& clientConnection, MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password);

            virtual void Connected() override;
            virtual void ClosingConnection() override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            virtual void HandleDataReceived() override;

        private:
            void Timeout();

        private:
            MqttClientObserverFactory& factory;
            infra::BoundedConstString clientId;
            infra::BoundedConstString username;
            infra::BoundedConstString password;
            infra::TimerSingleShot timeout;
            bool signaledFailure = false;
        };

        class StateConnected
            : public StateBase
        {
        public:
            using StateBase::StateBase;

            virtual void ClosingConnection() override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            virtual void HandleDataReceived() override;

            virtual void Publish() override;
            virtual void Subscribe() override;
            virtual void NotificationDone() override;

        protected:
            MqttClientImpl& ClientConnection() const;

        private:
            void HandlePubAck(infra::DataInputStream::WithErrorPolicy stream);
            void HandleSubAck(infra::DataInputStream::WithErrorPolicy stream);
            void HandlePublish(size_t packetLength, infra::DataInputStream::WithErrorPolicy stream);
            void ProcessSendOperations();

        private:
            class OperationBase
            {
            public:
                virtual ~OperationBase() = default;

                virtual void SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer) = 0;
                virtual std::size_t MessageSize(const MqttClientObserver& message) = 0;
            };

            class OperationPublish
                : public OperationBase
            {
            public:
                virtual void SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer) override;
                virtual std::size_t MessageSize(const MqttClientObserver& message) override;
            };

            class OperationSubscribe
                : public OperationBase
            {
            public:
                virtual void SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer) override;
                virtual std::size_t MessageSize(const MqttClientObserver& message) override;
            };

            class OperationPubAck
                : public OperationBase
            {
            public:
                virtual void SendStreamAvailable(MqttClientImpl::StateConnected& connectedState, infra::StreamWriter& writer) override;
                virtual std::size_t MessageSize(const MqttClientObserver& message) override;
            };

        private:
            infra::TimerSingleShot operationTimeout;
            uint16_t receivedPacketIdentifier;

            using OperationVariant = infra::PolymorphicVariant<OperationBase, OperationPublish, OperationSubscribe, OperationPubAck>;
            infra::BoundedDeque<OperationVariant>::WithMaxSize<2> sendOperations;
            bool executingSend = false;
            bool executingNotification = false;

        private:
            template<class operation>
            void QueueSendOperation()
            {
                sendOperations.emplace_back(infra::InPlaceType<operation>());
                ProcessSendOperations();
            }
        };

    protected:
        infra::SharedPtr<infra::StreamReader> ReceiveStream();

    private:
        infra::Duration operationTimeout;
        infra::PolymorphicVariant<StateBase, StateConnecting, StateConnected> state;
        infra::SharedPtr<MqttClientObserver> observer;
    };

    class MqttClientConnectorImpl
        : public MqttClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        MqttClientConnectorImpl(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password,
            infra::BoundedConstString hostname, uint16_t port, services::ConnectionFactoryWithNameResolver& connectionFactory);

        // Implementation of MqttClientConnector
        virtual void Connect(MqttClientObserverFactory& factory) override;
        virtual void CancelConnect() override;

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;
        
    private:
        infra::BoundedConstString hostname;
        uint16_t port;
        services::ConnectionFactoryWithNameResolver& connectionFactory;
        infra::BoundedConstString clientId;
        infra::BoundedConstString username;
        infra::BoundedConstString password;
        infra::SharedOptional<MqttClientImpl> client;
        MqttClientObserverFactory* clientObserverFactory = nullptr;
    };
}

#endif
