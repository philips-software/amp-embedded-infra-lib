#include "protobuf/echo/TracingEcho.hpp"

namespace services
{
    ServiceTracer::ServiceTracer(uint32_t serviceId)
        : serviceId(serviceId)
    {}

    uint32_t ServiceTracer::ServiceId() const
    {
        return serviceId;
    }

    TracingEchoOnConnection::TracingEchoOnConnection(services::Tracer& tracer, EchoErrorPolicy& errorPolicy)
        : EchoOnConnection(errorPolicy)
        , tracer(tracer)
    {}

    void TracingEchoOnConnection::AddServiceTracer(ServiceTracer& service)
    {
        services.push_front(service);
    }

    void TracingEchoOnConnection::RemoveServiceTracer(ServiceTracer& service)
    {
        services.erase_slow(service);
    }

    void TracingEchoOnConnection::ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents)
    {
        auto service = FindService(serviceId);

        if (service != nullptr)
            service->TraceMethod(methodId, contents, tracer);
        else
            tracer.Trace() << "Unknown service " << serviceId << " method " << methodId;

        EchoOnConnection::ExecuteMethod(serviceId, methodId, contents);
    }

    const ServiceTracer* TracingEchoOnConnection::FindService(uint32_t serviceId) const
    {
        for (auto& service : services)
            if (service.ServiceId() == serviceId)
                return &service;

        return nullptr;
    }

    void PrintField(bool value, services::Tracer& tracer)
    {
        tracer.Continue() << (value ? "true" : "false");
    }

    void PrintField(uint32_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(int32_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(uint64_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(int64_t value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(const infra::BoundedConstString& value, services::Tracer& tracer)
    {
        tracer.Continue() << value;
    }

    void PrintField(const infra::BoundedVector<uint8_t>& value, services::Tracer& tracer)
    {
        tracer.Continue() << "[";
        for (auto v : value)
            tracer.Continue() << infra::hex << infra::Width(2, 0) << v;
        tracer.Continue() << "]";
    }
}
