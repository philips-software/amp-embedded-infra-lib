#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_PARSER_HPP 
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_PARSER_HPP

#include "infra/stream/StringInputStream.hpp"
#include "services/cucumber/CucumberContext.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    class CucumberWireProtocolParser
    {
    public:
        CucumberWireProtocolParser() = default;

        enum class RequestType
        {
            StepMatches,
            Invoke,
            SnippetText,
            BeginScenario,
            EndScenario,
            Invalid
        };

        RequestType requestType;
        infra::JsonObject nameToMatch;
        uint32_t invokeId;
        infra::JsonArray invokeArguments;
        infra::Optional<infra::JsonObject> scenarioTags;

        void ParseRequest(const infra::BoundedString& inputString);
        bool Valid(const infra::BoundedString& inputString);

    private:
        void ParseStepMatchRequest(infra::JsonArray& input);
        void ParseInvokeRequest(infra::JsonArray& input);
        void ParseBeginScenarioRequest(infra::JsonArray& input);
        void ParseEndScenarioRequest();
        void ParseSnippetTextRequest();

        template <class T>
        T ConvertToIntType(const infra::BoundedString& input);
    };

    template<class T>
    inline T CucumberWireProtocolParser::ConvertToIntType(const infra::BoundedString& input)
    {
        T out{};
        infra::StringInputStream stream(input);
        stream >> out;
        return out;
    }
}

#endif
