#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "services/cucumber/CucumberWireProtocolFormatter.hpp"
#include "services/network/Connection.hpp"
#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    class CucumberWireProtocolConnectionObserver
        : public services::ConnectionObserver
    {
    public:
        CucumberWireProtocolConnectionObserver(infra::BoundedString& buffer, CucumberScenarioRequestHandler& scenarioRequestHandler);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    private:
        infra::BoundedString& buffer;

        CucumberWireProtocolParser parser;
        CucumberWireProtocolController controller;
        CucumberWireProtocolFormatter formatter;
    };

    class CucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        template<size_t BufferSize>
            using WithBuffer = infra::WithStorage<CucumberWireProtocolServer, infra::BoundedString::WithStorage<BufferSize>>;

        CucumberWireProtocolServer(infra::BoundedString& receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler);

    private:
        infra::BoundedString& receiveBuffer;
        CucumberScenarioRequestHandler& scenarioRequestHandler;
        infra::Creator<services::ConnectionObserver, CucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

namespace main_
{
    template<size_t BufferSize>
    struct CucumberInfrastructure
    {
        CucumberInfrastructure(services::ConnectionFactory& connectionFactory, uint16_t port)
            : server(connectionFactory, port, defaultScenarioRequestHandler)
        {}

        CucumberInfrastructure(services::ConnectionFactory& connectionFactory, uint16_t port, services::CucumberScenarioRequestHandler scenarioRequestHandler)
            : server(connectionFactory, port, scenarioRequestHandler)
        {}

        services::CucumberContext context;
        services::CucumberScenarioRequestHandler defaultScenarioRequestHandler;
        services::CucumberWireProtocolServer::WithBuffer<BufferSize> server;
    };
}

#endif
