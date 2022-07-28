#ifndef SERVICES_TRACING_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_TRACING_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/tracer/TracingOutputStream.hpp"

namespace services
{
    class TracingCucumberWireProtocolConnectionObserver
        : public CucumberWireProtocolConnectionObserver
    {
    public:
        TracingCucumberWireProtocolConnectionObserver(infra::BoundedString& receiveBuffer, CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    private:
        class TracingWriter
        {
        public:
            TracingWriter(infra::SharedPtr<infra::StreamWriter>&& writer, services::Tracer& tracer);

            infra::StreamWriter& Writer();

        private:
            infra::SharedPtr<infra::StreamWriter> writer;
            TracingStreamWriter tracingWriter;
        };

    private:
        services::Tracer& tracer;
        infra::SharedOptional<TracingWriter> tracingWriter;
    };

    class TracingCucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        template<size_t BufferSize>
            using WithBuffer = infra::WithStorage<TracingCucumberWireProtocolServer, infra::BoundedString::WithStorage<BufferSize>>;

        TracingCucumberWireProtocolServer(infra::BoundedString& receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer);

    private:
        services::Tracer& tracer;
        infra::BoundedString& receiveBuffer;
        CucumberScenarioRequestHandler& scenarioRequestHandler;
        infra::Creator<services::ConnectionObserver, TracingCucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

namespace main_
{
    template<size_t BufferSize>
    struct TracingCucumberInfrastructure
    {
        TracingCucumberInfrastructure(services::ConnectionFactory& connectionFactory, uint16_t port, services::Tracer& tracer)
            : server(connectionFactory, port, defaultScenarioRequestHandler, tracer)
        {}

        TracingCucumberInfrastructure(services::ConnectionFactory& connectionFactory, uint16_t port, services::CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer)
            : server(connectionFactory, port, scenarioRequestHandler, tracer)
        {}

        services::CucumberContext context;
        services::CucumberScenarioRequestHandler defaultScenarioRequestHandler;
        services::TracingCucumberWireProtocolServer::WithBuffer<BufferSize> server;
    };
}

#endif
