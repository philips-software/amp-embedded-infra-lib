#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_PARSER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_PARSER_HPP

#include "infra/stream/StringInputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonStreamingParser.hpp"

namespace services
{
    class CucumberWireProtocolParser
        : public infra::JsonArrayVisitor
    {
    public:
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
        infra::BoundedConstString nameToMatch;
        uint32_t invokeId;
        infra::JsonArray invokeArguments;
        infra::Optional<infra::JsonObject> scenarioTags;

        void ParseRequest(infra::BoundedConstString inputString);

        // Implementation of JsonArrayVisitor
        void VisitString(infra::BoundedConstString value) override;
        void ParseError() override;
        void SemanticError() override;
        void StringOverflow() override;

    private:
        void ParseStepMatchRequest(infra::JsonArray& input);
        void ParseInvokeRequest(infra::JsonArray& input);
        void ParseBeginScenarioRequest(infra::JsonArray& input);

        template<class T>
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
