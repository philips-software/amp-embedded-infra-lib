#include "services\cucumber\CucumberWireProtocolParser.hpp"

namespace services
{
    CucumberWireProtocolParser::CucumberWireProtocolParser(StepStorage& stepStorage)
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

    void CucumberWireProtocolParser::ParseRequest(const infra::ByteRange& inputRange)
    {
        infra::JsonArray input(infra::ByteRangeAsString(inputRange));

        for (auto value : input)
        {
        }

        if (input.Error())
        {
            requestType = invalid;
            return;
        }

        infra::JsonArrayIterator iterator(input.begin());

        if (iterator->Get<infra::JsonString>() == "step_matches")
        {
            requestType = step_matches;

            iterator++;
            nameToMatch = iterator->Get<infra::JsonObject>();

            infra::BoundedString::WithStorage<256> nameToMatchString;
            nameToMatch.GetString("name_to_match").ToString(nameToMatchString);

            matchResult = MatchName(nameToMatchString);
        }
        else if (iterator->Get<infra::JsonString>() == "begin_scenario")
            requestType = begin_scenario;
        else if (iterator->Get<infra::JsonString>() == "end_scenario")
            requestType = end_scenario;
        else if (iterator->Get<infra::JsonString>() == "invoke")
        {
            requestType = invoke;

            iterator++;
            infra::BoundedString::WithStorage<6> invokeIdString;
            iterator->Get<infra::JsonObject>().GetString("id").ToString(invokeIdString);
            infra::StringInputStream stream(invokeIdString);
            stream >> invokeId;

            invokeArguments = iterator->Get<infra::JsonObject>().GetArray("args");
        }
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

        if (this->requestType == step_matches)
        {
            if (matchResult == success)
            {
                {
                    infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);

                    result.Add((infra::BoundedConstString) "success");
                    infra::JsonArrayFormatter subArray(result.SubArray());

                    infra::JsonObjectFormatter subObject(subArray.SubObject());

                    infra::BoundedString::WithStorage<256> nameToMatchString;
                    nameToMatch.GetString("name_to_match").ToString(nameToMatchString);

                    infra::StringOutputStream::WithStorage<6> idStream;
                    idStream << stepStorage.MatchStep(nameToMatchString)->Id();
                    subObject.Add("id", idStream.Storage());
                    subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), stepStorage.MatchStep(nameToMatchString)->MatchArguments()) });
                }
                responseBuffer.insert(responseBuffer.size(), "\n");
            }
            else if (matchResult == duplicate)
                FailureMessage(responseBuffer, "Duplicate Step", "Exception.Step.Duplicate");
            else if (matchResult == fail)
                FailureMessage(responseBuffer, "Step not Matched", "Exception.Step.NotFound");
        }
        else if (this->requestType == begin_scenario)
            SuccessMessage(responseBuffer);
        else if (this->requestType == end_scenario)
            SuccessMessage(responseBuffer);
        else if (this->requestType == invoke)
        {
            if (Invoke())
            {
                SuccessMessage(responseBuffer);
            }
            else
            {
                FailureMessage(responseBuffer, "Invalid Arguments", "Exception.Arguments.Invalid");
            }
        }
        else if (this->requestType == snippet_text)
        {
            {
                infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
                result.Add((infra::BoundedConstString) "success");
                result.Add("snippet");
            }
            responseBuffer.insert(responseBuffer.size(), "\n");
        }
        else
            FailureMessage(responseBuffer, "Invalid Request", "Exception.InvalidRequestType");

        stream << infra::StringAsByteRange(responseBuffer);
    }

    CucumberWireProtocolParser::MatchResult CucumberWireProtocolParser::MatchName(const infra::BoundedString& nameToMatchString)
    {
        if (stepStorage.MatchStep(nameToMatchString) != infra::none)
            return MatchResult::success;
        else if (stepStorage.nrStepMatches >= 2)
            return MatchResult::duplicate;
        else
            return MatchResult::fail;
    }

    bool CucumberWireProtocolParser::Invoke()
    {
        if (!MatchArguments(invokeArguments))
        {
            return false;
        }
        else
        {
            stepStorage.MatchStep(invokeId)->Invoke(invokeArguments);
            return true;
        }
    }

    bool CucumberWireProtocolParser::MatchArguments(infra::JsonArray& arguments)
    {
        if (this->stepStorage.MatchStep(invokeId) != infra::none)
        {
            if (arguments.begin() == arguments.end() && (stepStorage.MatchStep(invokeId)->ContainsArguments() == false) && (stepStorage.MatchStep(invokeId)->TableHeaders().begin() == stepStorage.MatchStep(invokeId)->TableHeaders().end()))
                return true;

            infra::JsonArrayIterator argumentIterator(arguments.begin());

            uint8_t validStringCount = 0;
            for (auto string : JsonStringArray(arguments))
            {
                validStringCount++;
                argumentIterator++;
            }
            if (stepStorage.MatchStep(invokeId)->NrArguments() != validStringCount)
                return false;
            if ((argumentIterator == arguments.end() && stepStorage.MatchStep(invokeId)->TableHeaders() != infra::JsonArray("[]")) || (argumentIterator != arguments.end() && stepStorage.MatchStep(invokeId)->TableHeaders() == infra::JsonArray("[]")))
                return false;
            if (argumentIterator == arguments.end() && stepStorage.MatchStep(invokeId)->TableHeaders() == infra::JsonArray("[]"))
                return true;

            uint8_t rowCount = 0;
            uint8_t headerCollumns = 0;
            infra::JsonArrayIterator rowIterator(argumentIterator->Get<infra::JsonArray>().begin());
            while (rowIterator != argumentIterator->Get<infra::JsonArray>().end())
            {
                uint8_t collumnCount = 0;
                if (rowCount == 0)
                {
                    for (auto string : JsonStringArray(stepStorage.MatchStep(invokeId)->TableHeaders()))
                        headerCollumns++;
                    if (rowIterator->Get<infra::JsonArray>() != stepStorage.MatchStep(invokeId)->TableHeaders())
                        return false;
                }
                else
                {
                    for (auto string : JsonStringArray(rowIterator->Get<infra::JsonArray>()))
                        collumnCount++;
                    if (collumnCount != headerCollumns)
                        return false;
                }
                rowCount++;
                rowIterator++;
            }
            return true;
        }
        return false;
    }
}