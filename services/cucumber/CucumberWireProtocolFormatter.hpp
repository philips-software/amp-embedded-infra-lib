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

        void FormatResponse(infra::TextOutputStream::WithErrorPolicy& stream);

    private:
        void CreateSuccessMessage(infra::TextOutputStream::WithErrorPolicy& stream);
        void CreateSuccessMessage(infra::TextOutputStream::WithErrorPolicy& stream, uint32_t id, const infra::JsonArray& arguments, infra::BoundedConstString sourceLocation);
        void CreateFailureMessage(infra::TextOutputStream::WithErrorPolicy& stream, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

        void FormatStepMatchResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatInvokeResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatSnippetResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatBeginScenarioResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatEndScenarioResponse(infra::TextOutputStream::WithErrorPolicy& stream);

        void AddStringValue(infra::JsonArrayFormatter& formatter, const infra::BoundedString& nameToMatch, uint32_t& argPos, uint16_t& offset);
        void AddDigitValue(infra::JsonArrayFormatter& formatter, const infra::BoundedString& nameToMatch, uint32_t& argPos, uint16_t& offset);

        infra::JsonArray FormatStepArguments(const infra::BoundedString& nameToMatch);

    private:
        CucumberWireProtocolParser& parser;
        CucumberWireProtocolController& controller;
        infra::BoundedString::WithStorage<256> stepMatchArgumentsBuffer;
    };
}

#endif
