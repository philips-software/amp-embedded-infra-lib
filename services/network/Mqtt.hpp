#ifndef SERVICES_MQTT_HPP
#define SERVICES_MQTT_HPP

#include "infra/util/BoundedString.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class MqttClient;

    class MqttClientObserver
        : public infra::SharedOwnedObserver<MqttClientObserver, MqttClient>
    {
    public:
        virtual void Connected() {}
        virtual void PublishDone() = 0;
        virtual void SubscribeDone() = 0;
        virtual infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize) = 0;

        virtual void FillTopic(infra::StreamWriter& writer) const = 0;
        virtual void FillPayload(infra::StreamWriter& writer) const = 0;
    };

    class MqttClientObserverFactory
    {
    protected:
        MqttClientObserverFactory() = default;
        MqttClientObserverFactory(const MqttClientObserverFactory& other) = delete;
        MqttClientObserverFactory& operator=(const MqttClientObserverFactory& other) = delete;
        ~MqttClientObserverFactory() = default;

    public:
        enum ConnectFailReason
        {
            refused,
            connectionAllocationFailed,
            initializationFailed,
            initializationTimedOut
        };

        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<MqttClientObserver> client)>&& createdClientObserver) = 0;
        virtual void ConnectionFailed(ConnectFailReason reason) = 0;
    };

    class MqttClient
        : public infra::SharedOwningSubject<MqttClientObserver>
    {
    public:
        virtual void Publish() = 0;
        virtual void Subscribe() = 0;
        virtual void NotificationDone() = 0;
    };

    class MqttClientConnector
    {
    protected:
        MqttClientConnector() = default;
        MqttClientConnector(const MqttClientConnector& other) = delete;
        MqttClientConnector& operator=(const MqttClientConnector& other) = delete;
        ~MqttClientConnector() = default;

    public:
        virtual void Connect(MqttClientObserverFactory& factory) = 0;
        virtual void CancelConnect() = 0;
    };
}

#endif
