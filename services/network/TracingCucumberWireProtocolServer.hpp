#ifndef SERVICES_TRACING_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_TRACING_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "services/network/CucumberWireProtocolServer.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingCucumberWireProtocolConnectionObserver
        : public CucumberWireProtocolConnectionObserver
    {
    public:
        TracingCucumberWireProtocolConnectionObserver(services::Tracer& tracer);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    private:
        services::Tracer& tracer;
    };

    class TracingCucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        TracingCucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port, services::Tracer& tracer);

    private:
        services::Tracer& tracer;
        infra::Creator<services::ConnectionObserver, TracingCucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

#endif
