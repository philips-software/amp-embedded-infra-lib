#ifndef PROTOBUF_TRACING_ECHO_HPP
#define PROTOBUF_TRACING_ECHO_HPP

#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Echo.hpp"
#include "services/tracer/Tracer.hpp"
#include <type_traits>

namespace services
{
    class ServiceTracer
        : public infra::IntrusiveForwardList<ServiceTracer>::NodeType
    {
    public:
        explicit ServiceTracer(uint32_t serviceId);

        uint32_t ServiceId() const;
        virtual void TraceMethod(uint32_t methodId, infra::ProtoLengthDelimited& contents, services::Tracer& tracer) const = 0;

    private:
        uint32_t serviceId;
    };

    void PrintField(bool value, services::Tracer& tracer);
    void PrintField(uint32_t value, services::Tracer& tracer);
    void PrintField(int32_t value, services::Tracer& tracer);
    void PrintField(uint64_t value, services::Tracer& tracer);
    void PrintField(int64_t value, services::Tracer& tracer);
    void PrintField(const infra::BoundedConstString& value, services::Tracer& tracer);
    void PrintField(const infra::BoundedVector<uint8_t>& value, services::Tracer& tracer);

    template<class T>
    void PrintField(T value, services::Tracer& tracer, typename std::enable_if<std::is_enum<T>::value>::type* = 0)
    {
        PrintField(static_cast<uint32_t>(value), tracer);
    }

    template<std::size_t I, class T, class Enable>
    void PrintSubFields(const T& value, services::Tracer& tracer, typename T::template Type<I>* = 0)
    {
        PrintField(value.Get(std::integral_constant<uint32_t, I>()), tracer);
        PrintSubFields<I + 1>(value, tracer);
    }

    template<std::size_t I, class T>
    void PrintSubFields(const T& value, services::Tracer& tracer, ...)
    {}

    template<class T>
    void PrintField(const T& value, services::Tracer& tracer, typename T::template Type<0>* = 0)
    {
        tracer.Continue() << "{";
        PrintSubFields<0>(value, tracer);
        tracer.Continue() << "}";
    }

    template<class T>
    void PrintField(const infra::BoundedVector<T>& value, services::Tracer& tracer, typename T::template Type<0>* = 0)
    {
        tracer.Continue() << "[";
        for (auto& v : value)
        {
            if (&v != &value.front())
                tracer.Continue() << ", ";
            PrintField(v, tracer);
        }
        tracer.Continue() << "]";
    }
}

#endif
