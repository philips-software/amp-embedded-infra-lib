#include "services/cucumber/CucumberWireProtocolController.hpp"

namespace services
{
    CucumberWireProtocolController::CucumberWireProtocolController(ConnectionObserver& connectionObserver, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : invokeSuccess([this]() { InvokeSuccess(); })
        , invokeError([this](infra::BoundedConstString& reason) { InvokeError(reason); })
        , connectionObserver(connectionObserver)
        , scenarioRequestHandler(scenarioRequestHandler)
    {
        if (CucumberContext::InstanceSet())
        {
            CucumberContext::Instance().Add("InvokeSuccess", &invokeSuccess);
            CucumberContext::Instance().Add("InvokeError", &invokeError);
        }
    }

    void CucumberWireProtocolController::HandleRequest(CucumberWireProtocolParser& parser)
    {
        switch (parser.requestType)
        {
        case CucumberWireProtocolParser::RequestType::StepMatches:
            HandleStepMatchRequest(parser);
            break;
        case CucumberWireProtocolParser::RequestType::Invoke:
            HandleInvokeRequest(parser);
            break;
        case CucumberWireProtocolParser::RequestType::BeginScenario:
            HandleBeginScenarioRequest(parser);
            break;
        case CucumberWireProtocolParser::RequestType::EndScenario:
            HandleEndScenarioRequest();
            break;
        case CucumberWireProtocolParser::RequestType::SnippetText:
            HandleSnippetTextRequest();
            break;
        case CucumberWireProtocolParser::RequestType::Invalid:
            HandleInvalidRequest();
            break;
        default:
            break;
        }
    }

    void CucumberWireProtocolController::HandleStepMatchRequest(CucumberWireProtocolParser& parser)
    {
        parser.nameToMatch.GetString("name_to_match").ToString(nameToMatchString);
        storageMatch = CucumberStepStorage::Instance().MatchStep(nameToMatchString);
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
        for (const auto& string : JsonStringArray(arguments))
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
