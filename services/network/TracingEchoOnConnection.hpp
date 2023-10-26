#ifndef PROTOBUF_TRACING_ECHO_ON_CONNECTION_HPP
#define PROTOBUF_TRACING_ECHO_ON_CONNECTION_HPP

#include "protobuf/echo/TracingEcho.hpp"
#include "services/network/EchoOnConnection.hpp"

namespace services
{
    class TracingEchoOnConnection
        : public EchoOnConnection
        , private MethodSerializer
        , private MethodDeserializer
    {
    public:
        TracingEchoOnConnection(services::Tracer& tracer, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        void AddServiceTracer(ServiceTracer& service);
        void RemoveServiceTracer(ServiceTracer& service);

    protected:
        // Implementation of EchoOnConnection
        infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy) override;
        infra::SharedPtr<MethodDeserializer> StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const infra::SharedPtr<MethodDeserializer>& deserializer) override;

    private:
        // Implementation of MethodSerializer
        virtual bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void SerializationDone() override;

        // Implementation of MethodDeserializer
        virtual void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        virtual void ExecuteMethod() override;
        virtual bool Failed() const override;

    private:
        void SendingMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents) const;
        const ServiceTracer* FindService(uint32_t serviceId) const;

    private:
        class TracingWriter
            : public infra::StreamWriter
        {
        public:
            TracingWriter(infra::SharedPtr<infra::StreamWriter>&& delegate, TracingEchoOnConnection& echo);

            // Implementation of StreamWriter
            void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            std::size_t Available() const override;
            std::size_t ConstructSaveMarker() const override;
            std::size_t GetProcessedBytesSince(std::size_t marker) const override;
            infra::ByteRange SaveState(std::size_t marker) override;
            void RestoreState(infra::ByteRange range) override;
            infra::ByteRange Overwrite(std::size_t marker) override;

        private:
            TracingEchoOnConnection& echo;
            infra::SharedPtr<infra::StreamWriter> delegate;
            infra::BoundedVectorStreamWriter writer;
            infra::ByteRange savedRange;
            infra::ByteRange savedDelegateRange;
        };

    private:
        services::Tracer& tracer;
        infra::IntrusiveForwardList<ServiceTracer> services;

        infra::SharedOptional<TracingWriter> tracingWriter;
        infra::SharedPtr<MethodSerializer> serializer;
        infra::BoundedVector<uint8_t>::WithMaxSize<1024> writerBuffer;

        infra::SharedPtr<MethodDeserializer> deserializer;
        infra::BoundedVector<uint8_t>::WithMaxSize<1024> readerBuffer;
        const ServiceTracer* receivingService;
        uint32_t receivingMethodId;
    };
}

#endif
