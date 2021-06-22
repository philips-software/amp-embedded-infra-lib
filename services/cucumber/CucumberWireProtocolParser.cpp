#include "services/cucumber/CucumberWireProtocolParser.hpp"

namespace services
{
    void CucumberWireProtocolParser::ParseRequest(const infra::BoundedString& inputString)
    {
        infra::JsonArray input(inputString);
        infra::JsonArrayIterator iterator(input.begin());
        const auto& request = iterator->Get<infra::JsonString>();

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
            requestType = Invalid;
    }   

    bool CucumberWireProtocolParser::Valid(const infra::BoundedString& inputString)
    {
        infra::JsonArray input(inputString);
        for (auto& value : input);
        if (input.Error())
        {
            requestType = Invalid;
            return false;
        }
        return true;
    }

    void CucumberWireProtocolParser::ParseStepMatchRequest(infra::JsonArray& input)
    {
        requestType = Step_matches;
        infra::JsonArrayIterator iterator(input.begin());
        iterator++;
        nameToMatch = iterator->Get<infra::JsonObject>();
    }

    void CucumberWireProtocolParser::ParseInvokeRequest(infra::JsonArray& input)
    {
        requestType = Invoke;
        infra::JsonArrayIterator iterator(input.begin());
        iterator++;
        infra::BoundedString::WithStorage<6> idString;
        iterator->Get<infra::JsonObject>().GetString("id").ToString(idString);
        invokeId = ConvertToIntType<uint32_t>(idString);
        invokeArguments = iterator->Get<infra::JsonObject>().GetArray("args");
    }

    void CucumberWireProtocolParser::ParseBeginScenarioRequest(infra::JsonArray& input)
    {
        infra::JsonArrayIterator iterator(input.begin());
        if (++iterator != input.end())
            scenarioTags.Emplace(std::move(iterator->Get<infra::JsonObject>()));
        else
            scenarioTags = infra::none;
        requestType = Begin_scenario;
    }

    void CucumberWireProtocolParser::ParseEndScenarioRequest()
    {
        requestType = End_scenario;
    }

    void CucumberWireProtocolParser::ParseSnippetTextRequest()
    {
        requestType = Snippet_text;
    }
}
