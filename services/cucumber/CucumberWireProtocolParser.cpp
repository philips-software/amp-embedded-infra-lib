#include "services/cucumber/CucumberWireProtocolParser.hpp"

namespace services
{
    void CucumberWireProtocolParser::ParseRequest(infra::BoundedConstString inputString)
    {
        infra::JsonArray input(inputString);
        infra::JsonArrayIterator iterator(input.begin());
        const auto& request = std::get<infra::JsonString>(*iterator);

        if (request == "step_matches")
            ParseStepMatchRequest(input);
        else if (request == "begin_scenario")
            ParseBeginScenarioRequest(input);
        else if (request == "end_scenario")
            ParseEndScenarioRequest();
        else if (request == "invoke")
            ParseInvokeRequest(input);
        else if (request == "snippet_text")
            ParseSnippetTextRequest();
        else
            requestType = RequestType::Invalid;
    }

    bool CucumberWireProtocolParser::Valid(infra::BoundedConstString inputString)
    {
        infra::JsonArray input(inputString);
        for (const auto& value : input)
        {}

        if (input.Error())
        {
            requestType = RequestType::Invalid;
            return false;
        }

        return true;
    }

    void CucumberWireProtocolParser::ParseStepMatchRequest(infra::JsonArray& input)
    {
        requestType = RequestType::StepMatches;
        infra::JsonArrayIterator iterator(input.begin());
        iterator++;
        nameToMatch = std::get<infra::JsonObject>(*iterator);
    }

    void CucumberWireProtocolParser::ParseInvokeRequest(infra::JsonArray& input)
    {
        requestType = RequestType::Invoke;
        infra::JsonArrayIterator iterator(input.begin());
        iterator++;
        infra::BoundedString::WithStorage<6> idString;
        std::get<infra::JsonObject>(*iterator).GetString("id").ToString(idString);
        invokeId = ConvertToIntType<uint32_t>(idString);
        invokeArguments = std::get<infra::JsonObject>(*iterator).GetArray("args");
    }

    void CucumberWireProtocolParser::ParseBeginScenarioRequest(infra::JsonArray& input)
    {
        infra::JsonArrayIterator iterator(input.begin());
        if (++iterator != input.end())
            scenarioTags.emplace(std::move(std::get<infra::JsonObject>(*iterator)));
        else
            scenarioTags.reset();
        requestType = RequestType::BeginScenario;
    }

    void CucumberWireProtocolParser::ParseEndScenarioRequest()
    {
        requestType = RequestType::EndScenario;
    }

    void CucumberWireProtocolParser::ParseSnippetTextRequest()
    {
        requestType = RequestType::SnippetText;
    }
}
