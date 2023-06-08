#include "protobuf/echo/TracingEcho.hpp"
#include "infra/stream/ByteInputStream.hpp"

namespace services
{
    ServiceTracer::ServiceTracer(uint32_t serviceId)
        : serviceId(serviceId)
    {}

    uint32_t ServiceTracer::ServiceId() const
    {
        return serviceId;
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
