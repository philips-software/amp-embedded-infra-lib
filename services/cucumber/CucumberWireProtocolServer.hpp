#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "infra/syntax/JsonStreamingParser.hpp"
#include "services/cucumber/CucumberRequestHandlers.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"
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
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;

        struct InvokeInfo
        {
            bool successful;
            infra::Optional<infra::BoundedConstString> failReason;
        };

        void HandleRequest(CucumberWireProtocolParser& parser);
        void InvokeSuccess();
        void InvokeError(infra::BoundedConstString& reason);

    private:
        bool MatchArguments(std::size_t id, infra::JsonArray& arguments) const;

        void HandleStepMatchRequest(infra::BoundedConstString nameToMatch);
        void HandleInvokeRequest(CucumberWireProtocolParser& parser);
        void HandleBeginScenarioRequest(CucumberWireProtocolParser& parser);
        void HandleEndScenarioRequest();
        void HandleSnippetTextRequest();
        void HandleInvalidRequest();

    public:
        InvokeInfo invokeInfo;
        CucumberStepStorage::StepMatch storageMatch;
        infra::BoundedString::WithStorage<256> nameToMatchString;

    private:
        infra::BoundedString& buffer;
        CucumberScenarioRequestHandler& scenarioRequestHandler;
        CucumberWireProtocolParser parser;

    private:
        infra::JsonStreamingArrayParser::WithBuffers<32, 512, 2> streamingParser;
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
