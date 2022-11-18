#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_FORMATTER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_FORMATTER_HPP

#include "infra/stream/StringInputStream.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"

namespace services
{
    class CucumberWireProtocolFormatter
    {
    public:
        explicit CucumberWireProtocolFormatter(CucumberWireProtocolConnectionObserver& controller);

        void FormatResponse(CucumberWireProtocolParser::RequestType requestType, infra::TextOutputStream::WithErrorPolicy& stream);

    private:
        struct PositionAndOffset
        {
            std::size_t x;
            int16_t offset;

            std::size_t MatchPosition() const
            {
                return x + offset;
            }
        };

        void CreateSuccessMessage(infra::TextOutputStream::WithErrorPolicy& stream);
        void CreateSuccessMessage(infra::TextOutputStream::WithErrorPolicy& stream, uint32_t id, const infra::JsonArray& arguments, infra::BoundedConstString sourceLocation);
        void CreateFailureMessage(infra::TextOutputStream::WithErrorPolicy& stream, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

        void FormatStepMatchResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatInvokeResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatSnippetResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatBeginScenarioResponse(infra::TextOutputStream::WithErrorPolicy& stream);
        void FormatEndScenarioResponse(infra::TextOutputStream::WithErrorPolicy& stream);

        void AddStringValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, std::size_t& argPos, int16_t& offset);
        PositionAndOffset AddStringValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, PositionAndOffset previous);

        void AddDigitValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, std::size_t& argPos, int16_t& offset);
        void AddBooleanValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, std::size_t& argPos, int16_t& offset);

        infra::JsonArray FormatStepArguments(infra::BoundedConstString nameToMatch);

    private:
        CucumberWireProtocolConnectionObserver& controller;
        infra::BoundedString::WithStorage<1024> stepMatchArgumentsBuffer;
    };
}

#endif
