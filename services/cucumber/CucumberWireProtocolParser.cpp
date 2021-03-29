#include "services\cucumber\CucumberWireProtocolParser.hpp"

namespace services
{
    CucumberWireProtocolParser::CucumberWireProtocolParser(CucumberStepStorage& stepStorage)
        : invokeArguments(infra::JsonArray("[]"))
        , stepStorage(stepStorage)
    {}

    void CucumberWireProtocolParser::FailureMessage(infra::BoundedString& responseBuffer, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType)
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
            result.Add((infra::BoundedConstString) "fail");
            infra::JsonObjectFormatter subObject(result.SubObject());
            subObject.Add("message", failMessage);
            subObject.Add("exception", exceptionType);
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolParser::SuccessMessage(infra::BoundedString& responseBuffer)
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
            result.Add((infra::BoundedConstString) "success");
            infra::JsonArrayFormatter subArray(result.SubArray());
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolParser::SuccessMessage(uint8_t id, infra::JsonArray& arguments, infra::BoundedString& responseBuffer)
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);

            result.Add((infra::BoundedConstString) "success");
            infra::JsonArrayFormatter subArray(result.SubArray());
            infra::JsonObjectFormatter subObject(subArray.SubObject());

            infra::StringOutputStream::WithStorage<6> idStream;
            idStream << id;
            subObject.Add("id", idStream.Storage());
            subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), arguments) });
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }

    void CucumberWireProtocolParser::ParseRequest(const infra::ByteRange& inputRange)
    {
        infra::JsonArray input(infra::ByteRangeAsString(inputRange));
        if (InputError(input))
            return;

        infra::JsonArrayIterator iterator(input.begin());
        if (iterator->Get<infra::JsonString>() == "step_matches")
            ParseStepMatchRequest(iterator);
        else if (iterator->Get<infra::JsonString>() == "begin_scenario")
            requestType = begin_scenario;
        else if (iterator->Get<infra::JsonString>() == "end_scenario")
            requestType = end_scenario;
        else if (iterator->Get<infra::JsonString>() == "invoke")
            ParseInvokeRequest(iterator);
        else if (iterator->Get<infra::JsonString>() == "snippet_text")
            requestType = snippet_text;
        else
            requestType = invalid;
    }

    bool CucumberWireProtocolParser::ContainsArguments(const infra::BoundedString& string)
    {
        for (char& c : string)
        {
            infra::StringOutputStream::WithStorage<1> cToInt;
            cToInt << c;
            if (c == '\'' || (c >= '0' && c <= '9'))
                return true;
        }
        return false;
    }

    void CucumberWireProtocolParser::FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream)
    {
        infra::BoundedString::WithStorage<256> responseBuffer;

        switch (requestType)
        {
        case step_matches:
            FormatStepMatchResponse(responseBuffer);
            break;
        case invoke:
            FormatInvokeResponse(responseBuffer);
            break;
        case snippet_text:
            FormatSnippetResponse(responseBuffer);
            break;
        case begin_scenario:
            SuccessMessage(responseBuffer);
            break;
        case end_scenario:
            SuccessMessage(responseBuffer);
            break;
        case invalid:
            FailureMessage(responseBuffer, "Invalid Request", "Exception.InvalidRequestType");
            break;
        default:
            FailureMessage(responseBuffer, "Invalid Request", "Exception.InvalidRequestType");
            break;
        }  
        stream << infra::StringAsByteRange(responseBuffer);
    }

    CucumberWireProtocolParser::MatchResult CucumberWireProtocolParser::MatchName(const infra::BoundedString& nameToMatchString)
    {
        stepmatchStep = stepStorage.MatchStep(nameToMatchString);
        if (stepmatchStep != nullptr)
            return MatchResult::success;
        else if (stepStorage.nrStepMatches >= 2)
            return MatchResult::duplicate;
        else
            return MatchResult::fail;
    }

    bool CucumberWireProtocolParser::Invoke()
    {
        if (!MatchStringArguments(invokeArguments))
            return false;
        else
            return invokeStep->Invoke(invokeArguments);
    }

    bool CucumberWireProtocolParser::MatchStringArguments(infra::JsonArray& arguments)
    {
        if (invokeStep != nullptr)
        {
            if (arguments.begin() == arguments.end() && (invokeStep->ContainsStringArguments() == false))
                return true;

            infra::JsonArrayIterator argumentIterator(arguments.begin());

            uint8_t validStringCount = 0;
            for (auto string : JsonStringArray(arguments))
            {
                validStringCount++;
                argumentIterator++;
            }
            if (invokeStep->NrStringArguments() != validStringCount)
                return false;
            return true;
        }
        return false;
    }

    bool CucumberWireProtocolParser::InputError(infra::JsonArray& input)
    {
        for (auto value : input);
        if (input.Error())
        {
            requestType = invalid;
            return true;
        }
        return false;
    }

    void CucumberWireProtocolParser::ParseStepMatchRequest(infra::JsonArrayIterator& iteratorAtRequestType)
    {
        requestType = step_matches;
        iteratorAtRequestType++;
        nameToMatch = iteratorAtRequestType->Get<infra::JsonObject>();
        infra::BoundedString::WithStorage<256> nameToMatchString;
        nameToMatch.GetString("name_to_match").ToString(nameToMatchString);
        matchResult = MatchName(nameToMatchString);
    }

    void CucumberWireProtocolParser::ParseInvokeRequest(infra::JsonArrayIterator& iteratorAtRequestType)
    {
        requestType = invoke;
        iteratorAtRequestType++;
        infra::BoundedString::WithStorage<6> invokeIdString;
        iteratorAtRequestType->Get<infra::JsonObject>().GetString("id").ToString(invokeIdString);
        infra::StringInputStream stream(invokeIdString);
        stream >> invokeId;
        invokeArguments = iteratorAtRequestType->Get<infra::JsonObject>().GetArray("args");
    }

    void CucumberWireProtocolParser::FormatStepMatchResponse(infra::BoundedString& responseBuffer)
    {
        switch (matchResult)
        {
        case success:
            SuccessMessage(stepmatchStep->Id(), stepmatchStep->MatchArguments(), responseBuffer);
            break;
        case fail:
            FailureMessage(responseBuffer, "Step not Matched", "Exception.Step.NotFound");
            break;
        case duplicate:
            FailureMessage(responseBuffer, "Duplicate Step", "Exception.Step.Duplicate");
            break;
        default:
            break;
        }
    }

    void CucumberWireProtocolParser::FormatInvokeResponse(infra::BoundedString& responseBuffer)
    {
        invokeStep = stepStorage.MatchStep(invokeId);
        if (Invoke() && invokeStep != nullptr)
            SuccessMessage(responseBuffer);
        else
            FailureMessage(responseBuffer, "Invoke Failed", "Exception.Invoke.Failed");
    }

    void CucumberWireProtocolParser::FormatSnippetResponse(infra::BoundedString& responseBuffer)
    {
        {
            infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
            result.Add((infra::BoundedConstString) "success");
            result.Add("snippet");
        }
        responseBuffer.insert(responseBuffer.size(), "\n");
    }
}