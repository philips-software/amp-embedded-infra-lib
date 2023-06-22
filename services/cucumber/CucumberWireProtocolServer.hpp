#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "infra/syntax/JsonStreamingParser.hpp"
#include "services/cucumber/CucumberRequestHandler.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"
#include "services/network/Connection.hpp"
#include "services/network/SingleConnectionListener.hpp"
#include <string>

namespace services
{
    class CucumberWireProtocolConnectionObserver
        : public services::ConnectionObserver
    {
    public:
        CucumberWireProtocolConnectionObserver(CucumberScenarioRequestHandler& scenarioRequestHandler);

        // Implementation of ConnectionObserver
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;

        void HandleRequest(CucumberWireProtocolParser& parser);

    private:
        bool MatchArguments(std::size_t id, infra::JsonArray& arguments) const;

        void HandleStepMatchRequest(infra::BoundedConstString nameToMatch);
        void HandleInvokeRequest(CucumberWireProtocolParser& parser);
        void HandleBeginScenarioRequest(CucumberWireProtocolParser& parser);
        void HandleEndScenarioRequest();
        void HandleSnippetTextRequest();
        void HandleInvalidRequest();

    public:
        InvokeResult result;
        CucumberStepStorage::StepMatch storageMatch;
        infra::BoundedString::WithStorage<256> nameToMatchString;

    private:
        std::string buffer;
        CucumberScenarioRequestHandler& scenarioRequestHandler;
        CucumberWireProtocolParser parser;

    private:
        infra::JsonStreamingArrayParser::WithBuffers<32, 512, 2> streamingParser;
    };

    class CucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        CucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler);

        static void InitializeTestDriver();

    private:
        CucumberScenarioRequestHandler& scenarioRequestHandler;
        infra::Creator<services::ConnectionObserver, CucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

namespace main_
{
    struct CucumberInfrastructure
    {
        CucumberInfrastructure(services::ConnectionFactory& connectionFactory, uint16_t port)
            : server(connectionFactory, port, defaultScenarioRequestHandler)
        {}

        CucumberInfrastructure(services::ConnectionFactory& connectionFactory, uint16_t port, services::CucumberScenarioRequestHandler& scenarioRequestHandler)
            : server(connectionFactory, port, scenarioRequestHandler)
        {}

        services::CucumberScenarioRequestHandlerDefault defaultScenarioRequestHandler;
        services::CucumberWireProtocolServer server;
    };
}

#endif
