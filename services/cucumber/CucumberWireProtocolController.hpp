#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_CONTROLLER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_CONTROLLER_HPP

#include "services/cucumber/CucumberRequestHandlers.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class CucumberWireProtocolController
    {
    public:
        CucumberWireProtocolController(ConnectionObserver& connectionObserver, CucumberScenarioRequestHandler& scenarioRequestHandler);

        struct InvokeInfo
        {
            bool successful;
            infra::Optional<infra::BoundedConstString> failReason;
        };

        void HandleRequest(CucumberWireProtocolParser& parser);
        void InvokeSuccess();
        void InvokeError(infra::BoundedConstString& reason);

    private:
        bool MatchStringArguments(uint8_t id, infra::JsonArray& arguments);

        void HandleStepMatchRequest(CucumberWireProtocolParser& parser);
        void HandleInvokeRequest(CucumberWireProtocolParser& parser);
        void HandleBeginScenarioRequest(CucumberWireProtocolParser& parser);
        void HandleEndScenarioRequest();
        void HandleSnippetTextRequest();
        void HandleInvalidRequest();

    public:
        infra::AutoResetFunction<void()> invokeSuccess;
        infra::AutoResetFunction<void(infra::BoundedConstString&)> invokeError;
        InvokeInfo invokeInfo;
        CucumberStepStorage::Match storageMatch;
        infra::BoundedString::WithStorage<1024> nameToMatchString;

    private:
        ConnectionObserver& connectionObserver;
        CucumberScenarioRequestHandler& scenarioRequestHandler;
    };
}

#endif
