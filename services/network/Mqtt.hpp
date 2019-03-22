#ifndef SERVICES_MQTT_HPP
#define SERVICES_MQTT_HPP

#include "infra/util/BoundedString.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class MqttClient;

    class MqttClientObserver
        : public infra::SingleObserver<MqttClientObserver, MqttClient>
    {
    public:
        virtual void Connected() {}
        virtual void PublishDone() = 0;
        virtual void ClosingConnection() {}
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
        : public infra::Subject<MqttClientObserver>
    {
    public:
        virtual void Publish(infra::BoundedConstString topic, infra::BoundedConstString payload) = 0;
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
