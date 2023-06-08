#ifndef SERVICES_MQTT_IMPL_HPP
#define SERVICES_MQTT_IMPL_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/Mqtt.hpp"

namespace services
{
    class MqttClientImpl
        : public ConnectionObserver
        , public MqttClient
    {
    public:
        MqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password,
            infra::Duration operationTimeout = std::chrono::seconds(30), infra::Duration pingInterval = std::chrono::seconds(35));

        // Implementation of MqttClient
        void Publish() override;
        void Subscribe() override;
        void NotificationDone() override;
        void Disconnect() override;

        // Implementation of ConnectionObserver
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Attached() override;
        void Detaching() override;

        void CancellingConnection();

    protected:
        virtual infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize);

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
            struct InPlaceType
            {};

        public:
            explicit MqttFormatter(infra::DataOutputStream stream);

            void MessageConnect(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, infra::Duration keepAlive);
            static std::size_t MessageSizeConnect(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password);
            void MessagePublish(const MqttClientObserver& message, uint16_t packetIdentifier);
            static std::size_t MessageSizePublish(const MqttClientObserver& message);
            void MessageSubscribe(const MqttClientObserver& message);
            static std::size_t MessageSizeSubscribe(const MqttClientObserver& message);
            void MessagePubAck(const MqttClientObserver& message, uint16_t packetIdentifier);
            static std::size_t MessageSizePubAck(const MqttClientObserver& message);
            void MessagePing(const MqttClientObserver& message);
            static std::size_t MessageSizePing(const MqttClientObserver& message);

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
            explicit MqttParser(infra::DataInputStream stream);

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
            std::array<char, 4> protocolName{ { 'M', 'Q', 'T', 'T' } };
            uint8_t protocolLevel = 4;
            uint8_t connectFlags = 0x02; // clean session, no will
            infra::BigEndian<uint16_t> keepAlive = 0;
        };

        class StateBase
        {
        public:
            explicit StateBase(MqttClientImpl& clientConnection);
            StateBase(const StateBase& other) = delete;
            StateBase& operator=(const StateBase& other) = delete;
            virtual ~StateBase() = default;

        public:
            virtual void Connected();
            virtual void Detaching() = 0;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
            virtual void HandleDataReceived() = 0;

            virtual void Publish();
            virtual void Subscribe();
            virtual void NotificationDone();

            virtual void CancellingConnection();

        protected:
            MqttClientImpl& clientConnection;
        };

        class StateConnecting
            : public StateBase
        {
        public:
            StateConnecting(MqttClientImpl& clientConnection, MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password);

            void Connected() override;
            void Detaching() override;
            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            void HandleDataReceived() override;
            void CancellingConnection() override;

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
            explicit StateConnected(MqttClientImpl& clientConnection);

            void Detaching() override;
            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            void HandleDataReceived() override;

            void Publish() override;
            void Subscribe() override;
            void NotificationDone() override;

        protected:
            MqttClientImpl& ClientConnection() const;

        private:
            void HandleNotificationData(infra::DataInputStream& inputStream);
            void HandlePubAck(std::size_t packetLength, infra::DataInputStream::WithErrorPolicy stream);
            void HandleSubAck(std::size_t packetLength, infra::DataInputStream::WithErrorPolicy stream, infra::SharedPtr<infra::StreamReader>& reader);
            void HandlePublish(size_t packetLength, infra::DataInputStream::WithErrorPolicy stream);
            void PopFrontOperation();
            void ProcessSendOperations();
            void StartPing();
            void SendPing();
            void HandlePingReply();
            void StartWaitForPingReply();
            void PingReplyTimeout();
            uint16_t GeneratePacketIdentifier();

        private:
            class OperationBase
            {
            public:
                virtual ~OperationBase() = default;

                virtual void SendStreamAvailable(infra::StreamWriter& writer) = 0;
                virtual std::size_t MessageSize(const MqttClientObserver& message) = 0;

                virtual void HandlePubAck()
                {}

                virtual void HandleSubAck()
                {}
            };

            class OperationPublish
                : public OperationBase
            {
            public:
                OperationPublish(StateConnected& connectedState, MqttClientObserver& observer);

                void SendStreamAvailable(infra::StreamWriter& writer) override;
                std::size_t MessageSize(const MqttClientObserver& message) override;
                void HandlePubAck() override;

            private:
                StateConnected& connectedState;
                MqttClientObserver& observer;
            };

            class OperationSubscribe
                : public OperationBase
            {
            public:
                OperationSubscribe(StateConnected& connectedState, MqttClientObserver& observer);

                void SendStreamAvailable(infra::StreamWriter& writer) override;
                std::size_t MessageSize(const MqttClientObserver& message) override;
                void HandleSubAck() override;

            private:
                StateConnected& connectedState;
                MqttClientObserver& observer;
            };

            class OperationPubAck
                : public OperationBase
            {
            public:
                explicit OperationPubAck(StateConnected& connectedState);

                void SendStreamAvailable(infra::StreamWriter& writer) override;
                std::size_t MessageSize(const MqttClientObserver& message) override;

            private:
                StateConnected& connectedState;
            };

            class OperationPing
                : public OperationBase
            {
            public:
                explicit OperationPing(StateConnected& connectedState);

                void SendStreamAvailable(infra::StreamWriter& writer) override;
                std::size_t MessageSize(const MqttClientObserver& message) override;

            private:
                StateConnected& connectedState;
            };

        private:
            infra::TimerSingleShot operationTimeout;
            uint16_t receivedPacketIdentifier;

            using OperationVariant = infra::PolymorphicVariant<OperationBase, OperationPublish, OperationSubscribe, OperationPubAck, OperationPing>;
            infra::BoundedDeque<OperationVariant>::WithMaxSize<3> sendOperations;
            bool executingSend = false;
            bool executingNotification = false;
            uint16_t notificationPayloadSize = 0;
            infra::SharedPtr<infra::StreamWriter> notificationWriter;
            infra::TimerSingleShot pingTimer;
            uint16_t packetIdentifier = 0;
            bool waitingForPingReply = false;

        private:
            template<class operation, class... Args>
            void QueueSendOperation(Args&&... args)
            {
                sendOperations.emplace_back(infra::InPlaceType<operation>(), std::forward<Args>(args)...);
                ProcessSendOperations();
            }
        };

    protected:
        infra::SharedPtr<infra::StreamReader> ReceiveStream();

    private:
        infra::Duration operationTimeout;
        infra::Duration pingInterval;
        infra::PolymorphicVariant<StateBase, StateConnecting, StateConnected> state;
    };

    class MqttClientConnectorImpl
        : public MqttClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        MqttClientConnectorImpl(infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password,
            infra::BoundedConstString hostname, uint16_t port, services::ConnectionFactoryWithNameResolver& connectionFactory);

        void SetHostname(infra::BoundedConstString newHostname);
        void SetClientId(infra::BoundedConstString newClientId);

        void Stop(const infra::Function<void()>& onDone);

        // Implementation of MqttClientConnector
        void Connect(MqttClientObserverFactory& factory) override;
        void CancelConnect() override;

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    private:
        infra::BoundedConstString hostname;
        uint16_t port;
        bool connecting = false;
        services::ConnectionFactoryWithNameResolver& connectionFactory;
        infra::BoundedConstString clientId;
        infra::BoundedConstString username;
        infra::BoundedConstString password;
        infra::NotifyingSharedOptional<MqttClientImpl> client;
        MqttClientObserverFactory* clientObserverFactory = nullptr;
    };
}

#endif
