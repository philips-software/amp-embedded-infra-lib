#ifndef SERVICES_TRACING_MQTT_CLIENT_IMPL_HPP
#define SERVICES_TRACING_MQTT_CLIENT_IMPL_HPP

#include "services/network/MqttClientImpl.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingMqttClientImpl
        : public MqttClientImpl
    {
    public:
        TracingMqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, Tracer& tracer, infra::Duration operationTimeout = std::chrono::seconds(30));

        virtual void Publish() override;
        virtual void Subscribe() override;

    protected:
        virtual void ReceivedNotification(infra::BoundedConstString topic, infra::BoundedConstString payload) override;

    private:
        Tracer& tracer;
    };
}

#endif
