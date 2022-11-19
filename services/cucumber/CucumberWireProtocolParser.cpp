#include "services/cucumber/CucumberWireProtocolParser.hpp"

namespace services
{
    void CucumberWireProtocolParser::ParseRequest(infra::BoundedConstString inputString)
    {
        infra::JsonArray input(inputString);
        auto iterator(input.begin());
        const auto& request = iterator->Get<infra::JsonString>();

        if (request == "step_matches")
            ParseStepMatchRequest(input);
        else if (request == "begin_scenario")
            ParseBeginScenarioRequest(input);
        else if (request == "invoke")
            ParseInvokeRequest(input);
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

    void CucumberWireProtocolParser::VisitString(infra::BoundedConstString value)
    {
        if (value == "step_matches")
            ;
        else if (value == "begin_scenario")
            ;
        else if (value == "end_scenario")
            requestType = RequestType::EndScenario;
        else if (value == "invoke")
            ;
        else if (value == "snippet_text")
            requestType = RequestType::SnippetText;
        else
            requestType = RequestType::Invalid;
    }

    void CucumberWireProtocolParser::ParseStepMatchRequest(infra::JsonArray& input)
    {
        requestType = RequestType::StepMatches;
        auto iterator(input.begin());
        iterator++;
        nameToMatch = iterator->Get<infra::JsonObject>().GetString("name_to_match").Raw();
    }

    void CucumberWireProtocolParser::ParseInvokeRequest(infra::JsonArray& input)
    {
        requestType = RequestType::Invoke;
        auto iterator(input.begin());
        iterator++;
        infra::BoundedString::WithStorage<6> idString;
        iterator->Get<infra::JsonObject>().GetString("id").ToString(idString);
        invokeId = ConvertToIntType<uint32_t>(idString);
        invokeArguments = iterator->Get<infra::JsonObject>().GetArray("args");
    }

    void CucumberWireProtocolParser::ParseBeginScenarioRequest(infra::JsonArray& input)
    {
        auto iterator(input.begin());
        if (++iterator != input.end())
            scenarioTags.Emplace(std::move(iterator->Get<infra::JsonObject>()));
        else
            scenarioTags = infra::none;
        requestType = RequestType::BeginScenario;
    }
}
