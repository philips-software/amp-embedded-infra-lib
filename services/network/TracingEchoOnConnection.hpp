#ifndef PROTOBUF_TRACING_ECHO_ON_CONNECTION_HPP
#define PROTOBUF_TRACING_ECHO_ON_CONNECTION_HPP

#include "protobuf/echo/TracingEcho.hpp"
#include "services/network/EchoOnConnection.hpp"

namespace services
{
    class TracingEchoOnConnection
        : public EchoOnConnection
    {
    public:
        TracingEchoOnConnection(services::Tracer& tracer, EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);

        void AddServiceTracer(ServiceTracer& service);
        void RemoveServiceTracer(ServiceTracer& service);

    protected:
        virtual void ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, infra::StreamReaderWithRewinding& reader) override;
        virtual void SetStreamWriter(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        void SendingData(infra::ConstByteRange range) const;
        void SendingMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents) const;
        const ServiceTracer* FindService(uint32_t serviceId) const;

    private:
        class TracingWriter
            : public infra::StreamWriter
        {
        public:
            TracingWriter(infra::SharedPtr<infra::StreamWriter>&& delegate, TracingEchoOnConnection& echo);
            ~TracingWriter();

            virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
            virtual infra::ByteRange SaveState(std::size_t marker) override;
            virtual void RestoreState(infra::ByteRange range) override;
            virtual infra::ByteRange Overwrite(std::size_t marker) override;

        private:
            TracingEchoOnConnection& echo;
            infra::SharedPtr<infra::StreamWriter> delegate;
            infra::BoundedVectorStreamWriter::WithStorage<1024> writer;
            infra::ByteRange savedRange;
            infra::ByteRange savedDelegateRange;
        };

    private:
        services::Tracer& tracer;
        infra::IntrusiveForwardList<ServiceTracer> services;

        infra::SharedOptional<TracingWriter> tracingWriter;
    };
}

#endif
