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

    namespace detail
    {
        class TracingEchoOnStreamsDescendantHelper
            : private MethodSerializer
            , private MethodDeserializer
        {
        public:
            TracingEchoOnStreamsDescendantHelper(services::Tracer& tracer);

            void AddServiceTracer(ServiceTracer& service);
            void RemoveServiceTracer(ServiceTracer& service);

            infra::SharedPtr<MethodSerializer> GrantSend(const infra::SharedPtr<MethodSerializer>& serializer);
            infra::SharedPtr<MethodDeserializer> StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, infra::SharedPtr<MethodDeserializer>&& deserializer);
            void ReleaseDeserializer();

        private:
            // Implementation of MethodSerializer
            bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

            // Implementation of MethodDeserializer
            void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
            void ExecuteMethod() override;
            bool Failed() const override;

        private:
            void SendingMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents) const;
            const ServiceTracer* FindService(uint32_t serviceId) const;
            void SerializationDone();

        private:
            class TracingWriter
                : public infra::StreamWriter
            {
            public:
                TracingWriter(infra::SharedPtr<infra::StreamWriter>&& delegate, TracingEchoOnStreamsDescendantHelper& echo);

                // Implementation of StreamWriter
                void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
                std::size_t Available() const override;
                std::size_t ConstructSaveMarker() const override;
                std::size_t GetProcessedBytesSince(std::size_t marker) const override;
                infra::ByteRange SaveState(std::size_t marker) override;
                void RestoreState(infra::ByteRange range) override;
                infra::ByteRange Overwrite(std::size_t marker) override;

            private:
                infra::SharedPtr<infra::StreamWriter> delegate;
                infra::BoundedVectorStreamWriter writer;
                uint32_t& skipping;
                infra::ByteRange savedRange;
                infra::ByteRange savedDelegateRange;
            };

        private:
            services::Tracer& tracer;
            infra::IntrusiveForwardList<ServiceTracer> services;

            infra::NotifyingSharedOptional<TracingWriter> tracingWriter{ [this]()
                {
                    SerializationDone();
                } };
            ;
            infra::SharedPtr<MethodSerializer> serializer;
            infra::AccessedBySharedPtr serializerAccess{ [this]()
                {
                    serializer = nullptr;
                } };
            infra::BoundedVector<uint8_t>::WithMaxSize<1024> writerBuffer;
            uint32_t skipping = 0;

            infra::SharedPtr<MethodDeserializer> deserializer;
            infra::AccessedBySharedPtr deserializerAccess{ [this]()
                {
                    deserializer = nullptr;
                } };
            infra::BoundedVector<uint8_t>::WithMaxSize<1024> readerBuffer;
            const ServiceTracer* receivingService;
            uint32_t receivingMethodId;
        };
    }

    class TracingEchoOnStreams
    {
    public:
        TracingEchoOnStreams() = default;
        TracingEchoOnStreams(const TracingEchoOnStreams& other) = delete;
        TracingEchoOnStreams& operator=(const TracingEchoOnStreams& other) = delete;
        ~TracingEchoOnStreams() = default;

        virtual void AddServiceTracer(ServiceTracer& service) = 0;
        virtual void RemoveServiceTracer(ServiceTracer& service) = 0;
    };

    template<class Descendant>
    class TracingEchoOnStreamsDescendant
        : public TracingEchoOnStreams
        , public Descendant
    {
    public:
        template<class... Args>
        TracingEchoOnStreamsDescendant(services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy, services::Tracer& tracer, Args&&... args);

        // Implementation of TracingEchoOnStreams
        void AddServiceTracer(ServiceTracer& service) override;
        void RemoveServiceTracer(ServiceTracer& service) override;

    protected:
        // Implementation of EchoOnStreams
        infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy) override;
        infra::SharedPtr<MethodDeserializer> StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, infra::SharedPtr<MethodDeserializer>&& deserializer) override;
        void ReleaseDeserializer() override;

    private:
        detail::TracingEchoOnStreamsDescendantHelper helper;
    };

    //// Implementation

    template<class Descendant>
    template<class... Args>
    TracingEchoOnStreamsDescendant<Descendant>::TracingEchoOnStreamsDescendant(services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy, services::Tracer& tracer, Args&&... args)
        : Descendant(std::forward<Args>(args)..., serializerFactory, errorPolicy)
        , helper(tracer)
    {}

    template<class Descendant>
    void TracingEchoOnStreamsDescendant<Descendant>::AddServiceTracer(ServiceTracer& service)
    {
        helper.AddServiceTracer(service);
    }

    template<class Descendant>
    void TracingEchoOnStreamsDescendant<Descendant>::RemoveServiceTracer(ServiceTracer& service)
    {
        helper.RemoveServiceTracer(service);
    }

    template<class Descendant>
    infra::SharedPtr<MethodSerializer> TracingEchoOnStreamsDescendant<Descendant>::GrantSend(ServiceProxy& proxy)
    {
        return helper.GrantSend(Descendant::GrantSend(proxy));
    }

    template<class Descendant>
    infra::SharedPtr<MethodDeserializer> TracingEchoOnStreamsDescendant<Descendant>::StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, infra::SharedPtr<MethodDeserializer>&& deserializer)
    {
        return helper.StartingMethod(serviceId, methodId, size, std::move(deserializer));
    }

    template<class Descendant>
    void TracingEchoOnStreamsDescendant<Descendant>::ReleaseDeserializer()
    {
        EchoOnStreams::ReleaseDeserializer();
        helper.ReleaseDeserializer();
    }
}

#endif
