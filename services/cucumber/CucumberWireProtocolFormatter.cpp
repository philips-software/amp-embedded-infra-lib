#include "services/cucumber/CucumberWireProtocolFormatter.hpp"

namespace services
{
    CucumberWireProtocolFormatter::CucumberWireProtocolFormatter(CucumberWireProtocolParser& parser, CucumberWireProtocolController& controller)
        : parser(parser)
        , controller(controller)
    {}

    void CucumberWireProtocolFormatter::CreateFailureMessage(infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType)
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
            result.Add("fail");
            infra::JsonObjectFormatter subObject(result.SubObject());
            subObject.Add("message", failMessage);
            subObject.Add("exception", exceptionType);
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolFormatter::CreateSuccessMessage()
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
            result.Add("success");
            infra::JsonArrayFormatter subArray(result.SubArray());
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolFormatter::CreateSuccessMessage(uint32_t id, const infra::JsonArray& arguments)
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);

            result.Add("success");
            infra::JsonArrayFormatter subArray(result.SubArray());
            infra::JsonObjectFormatter subObject(subArray.SubObject());

            infra::StringOutputStream::WithStorage<6> idStream;
            idStream << id;
            subObject.Add("id", idStream.Storage());
            subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), arguments) });
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolFormatter::FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream)
    {
        responseBuffer.clear();

        switch (parser.requestType)
        {
        case CucumberWireProtocolParser::RequestType::StepMatches:
            FormatStepMatchResponse();
            break;
        case CucumberWireProtocolParser::RequestType::Invoke:
            FormatInvokeResponse();
            break;
        case CucumberWireProtocolParser::RequestType::SnippetText:
            FormatSnippetResponse();
            break;
        case CucumberWireProtocolParser::RequestType::BeginScenario:
            FormatBeginScenarioResponse();
            break;
        case CucumberWireProtocolParser::RequestType::EndScenario:
            FormatEndScenarioResponse();
            break;
        case CucumberWireProtocolParser::RequestType::Invalid:
            CreateFailureMessage("Invalid Request", "Exception.InvalidRequestType");
            break;
        default:
            std::abort();
            break;
        }
        stream << infra::StringAsByteRange(responseBuffer);
    }

    void CucumberWireProtocolFormatter::FormatStepMatchResponse()
    {
        switch (controller.storageMatch.result)
        {
        case CucumberStepStorage::StepMatchResult::Success:
            if (controller.storageMatch.step->HasStringArguments())
                CreateSuccessMessage(controller.storageMatch.id, FormatStepArguments(controller.nameToMatchString));
            else
                CreateSuccessMessage(controller.storageMatch.id, infra::JsonArray("[]"));
            break;
        case CucumberStepStorage::StepMatchResult::Fail:
            CreateFailureMessage("Step not Matched", "Exception.Step.NotFound");
            break;
        case CucumberStepStorage::StepMatchResult::Duplicate:
            CreateFailureMessage("Duplicate Step", "Exception.Step.Duplicate");
            break;
        default:
            break;
        }
    }

    void CucumberWireProtocolFormatter::FormatInvokeResponse()
    {
        if (controller.invokeInfo.successfull)
            CreateSuccessMessage();
        else
            CreateFailureMessage("Invoke Failed", *controller.invokeInfo.failReason);
    }

    void CucumberWireProtocolFormatter::FormatSnippetResponse()
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
            result.Add("success");
            result.Add("snippet");
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolFormatter::FormatBeginScenarioResponse()
    {
        CreateSuccessMessage();
    }

    void CucumberWireProtocolFormatter::FormatEndScenarioResponse()
    {
        CreateSuccessMessage();
    }

    void CucumberWireProtocolFormatter::AddStringValue(infra::JsonArrayFormatter& formatter, const infra::BoundedString& nameToMatch, uint32_t& argPos, uint16_t& offset)
    {
        infra::JsonObjectFormatter subObject(formatter.SubObject());
        infra::StringOutputStream::WithStorage<32> stringStream;
        for (uint8_t i = 1; nameToMatch[argPos + offset + i] != '\''; i++)
            stringStream << nameToMatch[argPos + offset + i];
        subObject.Add("val", stringStream.Storage());
        subObject.Add("pos", argPos + offset + 1);
        offset += (uint16_t)stringStream.Storage().size() - 2;
        argPos++;
    }

    void CucumberWireProtocolFormatter::AddDigitValue(infra::JsonArrayFormatter& formatter, const infra::BoundedString& nameToMatch, uint32_t& argPos, uint16_t& offset)
    {
        infra::JsonObjectFormatter subObject(formatter.SubObject());
        infra::StringOutputStream::WithStorage<32> digitStream;
        for (uint8_t i = 0; (nameToMatch[argPos + offset + i] >= '0' && nameToMatch[argPos + offset + i] <= '9'); i++)
            digitStream << nameToMatch[argPos + offset + i];
        subObject.Add("val", digitStream.Storage());
        subObject.Add("pos", argPos + offset);
        offset += (uint16_t)digitStream.Storage().size() - 2;
        argPos++;
    }

    infra::JsonArray CucumberWireProtocolFormatter::FormatStepArguments(const infra::BoundedString& nameToMatch)
    {
        {
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, stepMatchArgumentsBuffer);
            uint32_t strArgPos = controller.storageMatch.step->StepName().find("\'%s\'", 0);
            uint32_t intArgPos = controller.storageMatch.step->StepName().find("%d", 0);
            uint16_t argOffset = 0;
            while (strArgPos != infra::BoundedString::npos || intArgPos != infra::BoundedString::npos)
            {
                if (strArgPos != infra::BoundedString::npos && strArgPos < intArgPos)
                {
                    AddStringValue(arguments, nameToMatch, strArgPos, argOffset);
                    strArgPos = controller.storageMatch.step->StepName().find("\'%s\'", strArgPos);
                }
                else if (intArgPos != infra::BoundedString::npos && intArgPos < strArgPos)
                {
                    AddDigitValue(arguments, nameToMatch, intArgPos, argOffset);
                    intArgPos = controller.storageMatch.step->StepName().find("%d", intArgPos);
                }
            }
        }

        return infra::JsonArray(stepMatchArgumentsBuffer);
    }
}
