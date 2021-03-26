#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_PARSER_HPP 
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_PARSER_HPP

#include "services/cucumber/CucumberStepStorage.hpp"
#include "services/cucumber/CucumberStep.hpp"
#include "infra/stream/StringInputStream.hpp"

namespace services
{
    class CucumberWireProtocolParser
    {
    public:
        CucumberWireProtocolParser(CucumberStepStorage& stepStorage);

        enum RequestType
        {
            step_matches,
            invoke,
            snippet_text,
            begin_scenario,
            end_scenario,
            invalid
        };

        enum MatchResult
        {
            success,
            fail,
            duplicate
        };

    public:
        void ParseRequest(const infra::ByteRange& inputRange);
        void FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream);
        

    private:
        CucumberStepStorage& stepStorage;

        RequestType requestType;

        infra::JsonObject nameToMatch;
        MatchResult matchResult;

        uint8_t invokeId;
        infra::JsonArray invokeArguments;
        infra::BoundedString::WithStorage<256> invokeArgumentBuffer;

        services::CucumberStep* stepmatchStep;
        services::CucumberStep* invokeStep;

        bool ContainsArguments(const infra::BoundedString& string);
        MatchResult MatchName(const infra::BoundedString& nameToMatchString);
        bool Invoke();

        bool MatchStringArguments(infra::JsonArray& arguments);

        void SuccessMessage(infra::BoundedString& responseBuffer);
        void SuccessMessage(uint8_t id, infra::JsonArray& arguments, infra::BoundedString& responseBuffer);
        void FailureMessage(infra::BoundedString& responseBuffer, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

        bool InputError(infra::JsonArray& input);
        void ParseStepMatchRequest(infra::JsonArrayIterator& iteratorAtRequestType);
        void ParseInvokeRequest(infra::JsonArrayIterator& iteratorAtRequestType);

        void FormatStepMatchResponse(infra::BoundedString& responseBuffer);
        void FormatInvokeResponse(infra::BoundedString& responseBuffer);
        void FormatSnippetResponse(infra::BoundedString& responseBuffer);
    };
}

#endif