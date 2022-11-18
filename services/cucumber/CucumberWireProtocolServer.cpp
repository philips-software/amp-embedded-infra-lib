#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/cucumber/CucumberWireProtocolFormatter.hpp"

namespace services
{
    CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(infra::BoundedString& buffer, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : buffer(buffer)
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

    void CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        services::CucumberWireProtocolFormatter formatter(*this);
        infra::TextOutputStream::WithErrorPolicy stream(*writer);

        formatter.FormatResponse(parser.requestType, stream);
        writer = nullptr;
    }

    void CucumberWireProtocolConnectionObserver::DataReceived()
    {
        buffer.clear();

        infra::SharedPtr<infra::StreamReader> reader = Subject().ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        auto available = stream.Available();
        really_assert(available <= buffer.max_size());

        if (available > 0)
        {
            buffer.resize(available);
            stream >> buffer;
            Subject().AckReceived();

            if (parser.Valid(buffer))
                parser.ParseRequest(buffer);
            HandleRequest(parser);
        }
    }

    void CucumberWireProtocolConnectionObserver::HandleRequest(CucumberWireProtocolParser& parser)
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

    void CucumberWireProtocolConnectionObserver::HandleStepMatchRequest(infra::BoundedConstString nameToMatch)
    {
        storageMatch = CucumberStepStorage::Instance().MatchStep(nameToMatch);
        nameToMatchString = nameToMatch;
        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolConnectionObserver::HandleInvokeRequest(CucumberWireProtocolParser& parser)
    {
        auto& step = CucumberStepStorage::Instance().GetStep(parser.invokeId);
        if (parser.invokeArguments.begin() == parser.invokeArguments.end() && step.HasArguments() || MatchStringArguments(parser.invokeId, parser.invokeArguments))
            step.Invoke(parser.invokeArguments);
        else
            invokeInfo.successful = false;
    }

    void CucumberWireProtocolConnectionObserver::HandleBeginScenarioRequest(CucumberWireProtocolParser& parser)
    {
        scenarioRequestHandler.BeginScenario([this]()
            { Subject().RequestSendStream(Subject().MaxSendStreamSize()); });
    }

    void CucumberWireProtocolConnectionObserver::HandleEndScenarioRequest()
    {
        scenarioRequestHandler.EndScenario([this]()
            { Subject().RequestSendStream(Subject().MaxSendStreamSize()); });
    }

    void CucumberWireProtocolConnectionObserver::HandleSnippetTextRequest()
    {
        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolConnectionObserver::HandleInvalidRequest()
    {
        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    bool CucumberWireProtocolConnectionObserver::MatchStringArguments(uint32_t id, infra::JsonArray& arguments)
    {
        uint8_t validStringCount = 0;
        for (const auto& string : JsonStringArray(arguments))
            validStringCount++;
        return CucumberStepStorage::Instance().GetStep(id).NrArguments() == validStringCount;
    }

    void CucumberWireProtocolConnectionObserver::InvokeSuccess()
    {
        invokeInfo.successful = true;
        services::CucumberContext::Instance().TimeoutTimer().Cancel();

        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolConnectionObserver::InvokeError(infra::BoundedConstString& reason)
    {
        invokeInfo.successful = false;
        invokeInfo.failReason = reason;

        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(infra::BoundedString& receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , scenarioRequestHandler(scenarioRequestHandler)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address)
              {
            this->receiveBuffer.clear();
            value.Emplace(this->receiveBuffer, this->scenarioRequestHandler); })
    {}
}
