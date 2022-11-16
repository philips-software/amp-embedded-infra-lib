#include "services/cucumber/CucumberWireProtocolController.hpp"

namespace services
{
    CucumberWireProtocolController::CucumberWireProtocolController(ConnectionObserver& connectionObserver, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : connectionObserver(connectionObserver)
        , scenarioRequestHandler(scenarioRequestHandler)
    {
        really_assert(CucumberContext::InstanceSet());

        CucumberContext::Instance().onSuccess = [this]()
        {
            InvokeSuccess();
        };
        CucumberContext::Instance().onFailure = [this](infra::BoundedConstString& reason)
        {
            InvokeError(reason);
        };
    }

    void CucumberWireProtocolController::HandleRequest(CucumberWireProtocolParser& parser)
    {
        switch (parser.requestType)
        {
            case CucumberWireProtocolParser::RequestType::StepMatches:
                HandleStepMatchRequest(parser.nameToMatch);
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

    void CucumberWireProtocolController::HandleStepMatchRequest(infra::BoundedConstString nameToMatch)
    {
        storageMatch = CucumberStepStorage::Instance().MatchStep(nameToMatch);
        nameToMatchString = nameToMatch;
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::HandleInvokeRequest(CucumberWireProtocolParser& parser)
    {
        CucumberStep& step = CucumberStepStorage::Instance().GetStep(parser.invokeId);
        if (parser.invokeArguments.begin() == parser.invokeArguments.end() && step.HasStringArguments() || MatchStringArguments(parser.invokeId, parser.invokeArguments))
            step.Invoke(parser.invokeArguments);
        else
            invokeInfo.successful = false;
    }

    void CucumberWireProtocolController::HandleBeginScenarioRequest(CucumberWireProtocolParser& parser)
    {
        scenarioRequestHandler.BeginScenario([this]()
            { connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize()); });
    }

    void CucumberWireProtocolController::HandleEndScenarioRequest()
    {
        scenarioRequestHandler.EndScenario([this]()
            { connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize()); });
    }

    void CucumberWireProtocolController::HandleSnippetTextRequest()
    {
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::HandleInvalidRequest()
    {
        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    bool CucumberWireProtocolController::MatchStringArguments(uint32_t id, infra::JsonArray& arguments)
    {
        uint8_t validStringCount = 0;
        for (const auto& string : JsonStringArray(arguments))
            validStringCount++;
        return CucumberStepStorage::Instance().GetStep(id).NrArguments() == validStringCount;
    }

    void CucumberWireProtocolController::InvokeSuccess()
    {
        invokeInfo.successful = true;
        services::CucumberContext::Instance().TimeoutTimer().Cancel();

        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolController::InvokeError(infra::BoundedConstString& reason)
    {
        invokeInfo.successful = false;
        invokeInfo.failReason = reason;

        connectionObserver.Subject().RequestSendStream(connectionObserver.Subject().MaxSendStreamSize());
    }
}
