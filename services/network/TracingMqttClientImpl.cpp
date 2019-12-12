#include "services/network/TracingMqttClientImpl.hpp"

namespace services
{
    TracingMqttClientImpl::TracingMqttClientImpl(MqttClientObserverFactory& factory, infra::BoundedConstString clientId, infra::BoundedConstString username, infra::BoundedConstString password, Tracer& tracer, infra::Duration operationTimeout)
        : MqttClientImpl(factory, clientId, username, password, operationTimeout)
        , tracer(tracer)
    {
        tracer.Trace() << "MqttClient constructed";
    }

    TracingMqttClientImpl::~TracingMqttClientImpl()
    {
        tracer.Trace() << "MqttClient destructed";
    }

    void TracingMqttClientImpl::Publish()
    {
        tracer.Trace() << "MqttClient publish on topic ";
        GetObserver().FillTopic(tracer.Continue().Writer());
        tracer.Continue() << " with contents ";
        GetObserver().FillPayload(tracer.Continue().Writer());

        MqttClientImpl::Publish();
    }

    void TracingMqttClientImpl::Subscribe()
    {
        tracer.Trace() << "MqttClient subscribe on topic ";
        GetObserver().FillTopic(tracer.Continue().Writer());

        MqttClientImpl::Subscribe();
    }

    infra::SharedPtr<infra::StreamWriter> TracingMqttClientImpl::ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize)
    {
        tracer.Trace() << "MqttClient received notification on topic " << topic << " with contents size " << payloadSize;

        return MqttClientImpl::ReceivedNotification(topic, payloadSize);
    }
}
