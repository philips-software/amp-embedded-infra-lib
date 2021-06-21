#include "services/cucumber/CucumberWireProtocolController.hpp"

namespace services
{
    CucumberWireProtocolController::CucumberWireProtocolController(services::ConnectionObserver& connectionObserver
        , services::CucumberScenarioRequestHandler& scenarioRequestHandler)
        : connectionObserver(connectionObserver)
        , scenarioRequestHandler(scenarioRequestHandler)
        , invokeSuccess([this]() { InvokeSuccess(); })
        , invokeError([this](infra::BoundedConstString& reason) { InvokeError(reason); })
    {
        if (services::CucumberContext::InstanceSet())
            if (!services::CucumberContext::Instance().Contains("InvokeSuccess"))
                services::CucumberContext::Instance().Add("InvokeSuccess", &invokeSuccess);
            if (!services::CucumberContext::Instance().Contains("InvokeError"))
                services::CucumberContext::Instance().Add("InvokeError", &invokeError);
    }

    void CucumberWireProtocolController::HandleRequest(CucumberWireProtocolParser& parser)
    {
        switch (parser.requestType)
        {
        case CucumberWireProtocolParser::Step_matches:
            HandleStepMatchRequest(parser);
            break;
        case CucumberWireProtocolParser::Invoke:
            HandleInvokeRequest(parser);
            break;
        case CucumberWireProtocolParser::Begin_scenario:
            HandleBeginScenarioRequest(parser);
            break;
        case CucumberWireProtocolParser::End_scenario:
            HandleEndScenarioRequest();
            break;
        case CucumberWireProtocolParser::Snippet_text:
            HandleSnippetTextRequest();
            break;
        case CucumberWireProtocolParser::Invalid:
            HandleInvalidRequest();
        default:
            break;
        }
    }

    void CucumberWireProtocolController::HandleStepMatchRequest(CucumberWireProtocolParser& parser)
    {
        parser.nameToMatch.GetString("name_to_match").ToString(nameToMatchString);
        storageMatch = services::CucumberStepStorage::Instance().MatchStep(nameToMatchString);
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::HandleInvokeRequest(CucumberWireProtocolParser& parser)
    {
        CucumberStep& step = CucumberStepStorage::Instance().GetStep(parser.invokeId);
        if (parser.invokeArguments.begin() == parser.invokeArguments.end() && step.HasStringArguments() || MatchStringArguments(parser.invokeId, parser.invokeArguments))
            step.Invoke(parser.invokeArguments);
        else
            invokeInfo.successfull = false;
    }

    void CucumberWireProtocolController::HandleBeginScenarioRequest(CucumberWireProtocolParser& parser)
    {
        scenarioRequestHandler.BeginScenario();
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::HandleEndScenarioRequest()
    {
        scenarioRequestHandler.EndScenario();
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::HandleSnippetTextRequest()
    {
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::HandleInvalidRequest()
    {
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    bool CucumberWireProtocolController::MatchStringArguments(uint8_t id, infra::JsonArray& arguments)
    {
        uint8_t validStringCount = 0;
        for (auto string : JsonStringArray(arguments))
            validStringCount++;
        return CucumberStepStorage::Instance().GetStep(id).NrArguments() == validStringCount;
    }

    void CucumberWireProtocolController::InvokeSuccess()
    {
        invokeInfo.successfull = true;
   	    services::CucumberContext::Instance().TimeoutTimer().Cancel();
        
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::InvokeError(infra::BoundedConstString& reason)
    {
        invokeInfo.successfull = false;
        invokeInfo.failReason = reason;

        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }
}
