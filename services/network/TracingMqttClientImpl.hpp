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
        ~TracingMqttClientImpl();

        void Publish() override;
        void Subscribe() override;

    protected:
        infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize) override;

    private:
        Tracer& tracer;
    };
}

#endif
