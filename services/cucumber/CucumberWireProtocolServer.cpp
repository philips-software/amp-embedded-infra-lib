#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "infra/stream/StdStringInputStream.hpp"
#include "infra/util/StaticStorage.hpp"
#include "services/cucumber/CucumberWireProtocolFormatter.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"

#if EMIL_CUCUMBER_USE_GMOCK
#include "gmock/gmock.h"

namespace
{
    void InitializeGoogleMock()
    {
        ::testing::GTEST_FLAG(throw_on_failure) = true;
        ::testing::InitGoogleMock();
    }
}
#endif

namespace services
{
    CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(CucumberScenarioRequestHandler& scenarioRequestHandler)
        : scenarioRequestHandler(scenarioRequestHandler)
        , streamingParser(parser)
    {}

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
        infra::ReConstruct(parser);
        infra::ReConstruct(streamingParser, parser);

        infra::SharedPtr<infra::StreamReader> reader = Subject().ReceiveStream();
        infra::StdStringInputStream::WithErrorPolicy stream(*reader);

        auto available = stream.Available();
        really_assert(available <= buffer.max_size());

        if (available > 0)
        {
            buffer.resize(available);
            stream >> buffer;
            streamingParser.Feed(buffer);
            Subject().AckReceived();

            if (parser.requestType != CucumberWireProtocolParser::RequestType::Invalid)
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
        if (MatchArguments(parser.invokeId, parser.invokeArguments))
            result = step.Invoke(parser.invokeArguments);

        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolConnectionObserver::HandleBeginScenarioRequest(CucumberWireProtocolParser& parser)
    {
        scenarioRequestHandler.BeginScenario([this]()
            {
                Subject().RequestSendStream(Subject().MaxSendStreamSize());
            });
    }

    void CucumberWireProtocolConnectionObserver::HandleEndScenarioRequest()
    {
        scenarioRequestHandler.EndScenario([this]()
            {
                Subject().RequestSendStream(Subject().MaxSendStreamSize());
            });
    }

    void CucumberWireProtocolConnectionObserver::HandleSnippetTextRequest()
    {
        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    void CucumberWireProtocolConnectionObserver::HandleInvalidRequest()
    {
        Subject().RequestSendStream(Subject().MaxSendStreamSize());
    }

    bool CucumberWireProtocolConnectionObserver::MatchArguments(std::size_t id, infra::JsonArray& arguments) const
    {
        std::size_t validArgumentCount = 0;
        for (const auto& string : JsonStringArray(arguments))
            ++validArgumentCount;
        return CucumberStepStorage::Instance().GetStep(id).NrArguments() == validArgumentCount;
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , scenarioRequestHandler(scenarioRequestHandler)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address)
              {
                  value.Emplace(this->scenarioRequestHandler);
              })
    {
        InitializeTestDriver();
    }

    void CucumberWireProtocolServer::InitializeTestDriver()
    {
#ifdef EMIL_CUCUMBER_USE_GMOCK
        InitializeGoogleMock();
#endif
    }
}
