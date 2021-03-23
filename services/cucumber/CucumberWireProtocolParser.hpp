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
        CucumberWireProtocolParser(StepStorage& stepStorage);

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
        bool ContainsArguments(const infra::BoundedString& string);
        MatchResult MatchName(const infra::BoundedString& nameToMatchString);
        bool Invoke();

        void FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream);
        bool MatchArguments(infra::JsonArray& arguments);

        void SuccessMessage(infra::BoundedString& responseBuffer);
        void FailureMessage(infra::BoundedString& responseBuffer, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

    private:
        StepStorage& stepStorage;

        RequestType requestType;

        infra::JsonObject nameToMatch;
        MatchResult matchResult;

        uint8_t invokeId;
        infra::JsonArray invokeArguments;
        infra::BoundedString::WithStorage<256> invokeArgumentBuffer;
    };
}

#endif