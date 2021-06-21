#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_FORMATTER_HPP 
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_FORMATTER_HPP

#include "services/cucumber/CucumberWireProtocolController.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/stream/StringInputStream.hpp"

namespace services
{
    class CucumberWireProtocolFormatter
    {
    public:
        CucumberWireProtocolFormatter(CucumberWireProtocolParser& parser, CucumberWireProtocolController& controller);

    public:
        void FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream);

    private:
        CucumberWireProtocolParser& parser;
        CucumberWireProtocolController& controller;

        infra::BoundedString::WithStorage<256> responseBuffer;
        infra::BoundedString::WithStorage<256> stepMatchArgumentsBuffer;

        void CreateSuccessMessage();
        void CreateSuccessMessage(uint8_t id, const infra::JsonArray& arguments);
        void CreateFailureMessage(infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

        void FormatStepMatchResponse();
        void FormatInvokeResponse();
        void FormatSnippetResponse();
        void FormatBeginScenarioResponse();
        void FormatEndScenarioResponse();

        void AddStringValue(infra::JsonArrayFormatter& formatter, const infra::BoundedString& nameToMatch, uint32_t& argPos, uint16_t& offset);
        void AddDigitValue(infra::JsonArrayFormatter& formatter, const infra::BoundedString& nameToMatch, uint32_t& argPos, uint16_t& offset);

        infra::JsonArray FormatStepArguments(const infra::BoundedString& nameToMatch);
    };
}

#endif
