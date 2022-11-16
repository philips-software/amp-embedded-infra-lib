#include "services/cucumber/CucumberWireProtocolFormatter.hpp"

namespace services
{
    CucumberWireProtocolFormatter::CucumberWireProtocolFormatter(CucumberWireProtocolController& controller)
        : controller(controller)
    {}

    void CucumberWireProtocolFormatter::FormatResponse(CucumberWireProtocolParser::RequestType requestType, infra::TextOutputStream::WithErrorPolicy& stream)
    {
        switch (requestType)
        {
            case CucumberWireProtocolParser::RequestType::StepMatches:
                FormatStepMatchResponse(stream);
                break;
            case CucumberWireProtocolParser::RequestType::Invoke:
                FormatInvokeResponse(stream);
                break;
            case CucumberWireProtocolParser::RequestType::SnippetText:
                FormatSnippetResponse(stream);
                break;
            case CucumberWireProtocolParser::RequestType::BeginScenario:
                FormatBeginScenarioResponse(stream);
                break;
            case CucumberWireProtocolParser::RequestType::EndScenario:
                FormatEndScenarioResponse(stream);
                break;
            case CucumberWireProtocolParser::RequestType::Invalid:
                CreateFailureMessage(stream, "Invalid Request", "Exception.InvalidRequestType");
                break;
            default:
                std::abort();
        }
    }

    void CucumberWireProtocolFormatter::CreateFailureMessage(infra::TextOutputStream::WithErrorPolicy& stream, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType)
    {
        {
            infra::JsonArrayFormatter result(stream);
            result.Add("fail");
            infra::JsonObjectFormatter subObject(result.SubObject());
            subObject.Add("message", failMessage);
            subObject.Add("exception", exceptionType);
        }

        stream << "\n";
    }

    void CucumberWireProtocolFormatter::CreateSuccessMessage(infra::TextOutputStream::WithErrorPolicy& stream)
    {
        {
            infra::JsonArrayFormatter result(stream);
            result.Add("success");
            infra::JsonArrayFormatter subArray(result.SubArray());
        }

        stream << "\n";
    }

    void CucumberWireProtocolFormatter::CreateSuccessMessage(infra::TextOutputStream::WithErrorPolicy& stream, uint32_t id, const infra::JsonArray& arguments, infra::BoundedConstString sourceLocation)
    {
        {
            infra::JsonArrayFormatter result(stream);

            result.Add("success");
            infra::JsonArrayFormatter subArray(result.SubArray());
            infra::JsonObjectFormatter subObject(subArray.SubObject());

            infra::StringOutputStream::WithStorage<6> idStream;
            idStream << id;
            subObject.Add("id", idStream.Storage());
            subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), arguments) });
            subObject.Add("source", sourceLocation);
        }

        stream << "\n";
    }

    void CucumberWireProtocolFormatter::FormatStepMatchResponse(infra::TextOutputStream::WithErrorPolicy& stream)
    {
        switch (controller.storageMatch.result)
        {
            case CucumberStepStorage::StepMatchResult::Success:
                if (controller.storageMatch.step->HasStringArguments())
                    CreateSuccessMessage(stream, controller.storageMatch.id, FormatStepArguments(controller.nameToMatchString), controller.storageMatch.step->SourceLocation());
                else
                    CreateSuccessMessage(stream, controller.storageMatch.id, infra::JsonArray("[]"), controller.storageMatch.step->SourceLocation());
                break;
            case CucumberStepStorage::StepMatchResult::Fail:
                CreateFailureMessage(stream, "Step not Matched", "Exception.Step.NotFound");
                break;
            case CucumberStepStorage::StepMatchResult::Duplicate:
                CreateFailureMessage(stream, "Duplicate Step", "Exception.Step.Duplicate");
                break;
            default:
                break;
        }
    }

    void CucumberWireProtocolFormatter::FormatInvokeResponse(infra::TextOutputStream::WithErrorPolicy& stream)
    {
        if (controller.invokeInfo.successful)
            CreateSuccessMessage(stream);
        else
            CreateFailureMessage(stream, "Invoke Failed", *controller.invokeInfo.failReason);
    }

    void CucumberWireProtocolFormatter::FormatSnippetResponse(infra::TextOutputStream::WithErrorPolicy& stream)
    {
        {
            infra::JsonArrayFormatter result(stream);
            result.Add("success");
            result.Add("snippet");
        }
        stream << "\n";
    }

    void CucumberWireProtocolFormatter::FormatBeginScenarioResponse(infra::TextOutputStream::WithErrorPolicy& stream)
    {
        CreateSuccessMessage(stream);
    }

    void CucumberWireProtocolFormatter::FormatEndScenarioResponse(infra::TextOutputStream::WithErrorPolicy& stream)
    {
        CreateSuccessMessage(stream);
    }

    void CucumberWireProtocolFormatter::AddStringValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, std::size_t& argPos, int16_t& offset)
    {
        PositionAndOffset input { argPos, offset };
        auto result = AddStringValue(formatter, nameToMatch, input);
        argPos = result.x;
        offset = result.offset;
    }

    CucumberWireProtocolFormatter::PositionAndOffset CucumberWireProtocolFormatter::AddStringValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, PositionAndOffset previous)
    {
        assert(nameToMatch[previous.MatchPosition()] == '\'');
        infra::JsonObjectFormatter subObject(formatter.SubObject());
        infra::StringOutputStream::WithStorage<1024> stringStream;

        for (std::size_t i = 1; nameToMatch[previous.MatchPosition() + i] != '\''; ++i)
            stringStream << nameToMatch[previous.MatchPosition() + i];

        subObject.Add("val", stringStream.Storage());
        subObject.Add("pos", previous.MatchPosition() + 1);

        return { previous.x + 1, static_cast<int16_t>(previous.offset + stringStream.Storage().size() - 2) };
    }

    void CucumberWireProtocolFormatter::AddDigitValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, std::size_t& argPos, int16_t& offset)
    {
        infra::JsonObjectFormatter subObject(formatter.SubObject());
        infra::StringOutputStream::WithStorage<10> digitStream;
        for (std::size_t i = 0; argPos + offset + i < nameToMatch.size() && (nameToMatch[argPos + offset + i] >= '0' && nameToMatch[argPos + offset + i] <= '9'); ++i)
            digitStream << nameToMatch[argPos + offset + i];
        subObject.Add("val", digitStream.Storage());
        subObject.Add("pos", argPos + offset);
        offset += (int16_t)digitStream.Storage().size() - 2;
        ++argPos;
    }

    void CucumberWireProtocolFormatter::AddBooleanValue(infra::JsonArrayFormatter& formatter, infra::BoundedConstString nameToMatch, std::size_t& argPos, int16_t& offset)
    {
        infra::JsonObjectFormatter subObject(formatter.SubObject());
        infra::StringOutputStream::WithStorage<5> boolStream;
        if (nameToMatch[argPos + offset] == 't')
            boolStream << "true";
        else if (nameToMatch[argPos + offset] == 'f')
            boolStream << "false";
        subObject.Add("val", boolStream.Storage());
        subObject.Add("pos", argPos + offset);
        offset += (int16_t)boolStream.Storage().size() - 2;
        ++argPos;
    }

    infra::JsonArray CucumberWireProtocolFormatter::FormatStepArguments(infra::BoundedConstString nameToMatch)
    {
        stepMatchArgumentsBuffer.clear();
        {
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, stepMatchArgumentsBuffer);
            auto strArgPos = controller.storageMatch.step->StepName().find(R"('%s')", 0);
            auto intArgPos = controller.storageMatch.step->StepName().find("%d", 0);
            auto boolArgPos = controller.storageMatch.step->StepName().find("%b", 0);
            int16_t argOffset = 0;
            while (strArgPos != infra::BoundedString::npos || intArgPos != infra::BoundedString::npos || boolArgPos != infra::BoundedString::npos)
            {
                auto nArgument = std::min({ strArgPos, intArgPos, boolArgPos });
                if (strArgPos != infra::BoundedString::npos && strArgPos == nArgument)
                {
                    AddStringValue(arguments, nameToMatch, strArgPos, argOffset);
                    strArgPos = controller.storageMatch.step->StepName().find(R"('%s')", strArgPos);
                }
                else if (intArgPos != infra::BoundedString::npos && intArgPos == nArgument)
                {
                    AddDigitValue(arguments, nameToMatch, intArgPos, argOffset);
                    intArgPos = controller.storageMatch.step->StepName().find("%d", intArgPos);
                }
                else if (boolArgPos != infra::BoundedString::npos && boolArgPos == nArgument)
                {
                    AddBooleanValue(arguments, nameToMatch, boolArgPos, argOffset);
                    boolArgPos = controller.storageMatch.step->StepName().find("%b", boolArgPos);
                }
            }
        }

        return infra::JsonArray(stepMatchArgumentsBuffer);
    }
}
