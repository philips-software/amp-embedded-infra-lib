#include "services/network/CucumberWireProtocolServer.hpp"

namespace services
{
    uint8_t StepStorage::Step::nrSteps = 0;

    uint8_t StepStorage::Step::Id()
    {
        return id;
    }

    void StepStorage::Step::SetId(uint8_t id)
    {
        this->id = id;
    }

    infra::JsonArray StepStorage::Step::MatchArguments()
    {
        return matchArguments;
    }

    void StepStorage::Step::SetMatchArguments(infra::JsonArray arguments)
    {
        this->matchArguments = arguments;
    }

    infra::JsonArray StepStorage::Step::TableHeaders()
    {
        return this->tableHeaders;
    }

    void StepStorage::Step::SetTableHeaders(infra::JsonArray arguments)
    {
        this->tableHeaders = arguments;
    }

    infra::BoundedString StepStorage::Step::StepName()
    {
        return stepName;
    }

    void StepStorage::Step::StepName(infra::BoundedString stepName)
    {
        this->stepName = stepName;
    }

    infra::BoundedString StepStorage::Step::MatchArgumentsBuffer()
    {
        return matchArgumentsBuffer;
    }

    bool StepStorage::Step::ContainsArguments()
    {
        if (this->StepName().find("\'%s\'") != infra::BoundedString::npos || this->StepName().find("%d") != infra::BoundedString::npos)
            return true;
        return false;
    }

    uint8_t StepStorage::Step::NrArguments()
    {
        uint8_t nrArguments = 0;
        {
            uint8_t argPos = 0;
            do
            {
                if (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("\'%s\'", argPos);
                    argPos++;
                    nrArguments++;
                }
                if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("%d", argPos);
                    argPos++;
                    nrArguments++;
                }
            } while (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos);
        }
        return nrArguments;
    }

    infra::JsonArray StepStorage::Step::ParseArguments(const infra::BoundedString& nameToMatch, infra::BoundedString& arrayBuffer)
    {
        {
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, arrayBuffer);
            uint8_t argPos = 0;
            uint8_t argOffset = 0;
            do
            {
                if (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("\'%s\'", argPos);
                    if (nameToMatch[argPos + argOffset] == '\'')
                    {
                        infra::JsonObjectFormatter subObject(arguments.SubObject());
                        infra::StringOutputStream::WithStorage<32> stringStream;
                        for (uint8_t i = 1; nameToMatch[argPos + argOffset + i] != '\''; i++)
                            stringStream << nameToMatch[argPos + argOffset + i];
                        subObject.Add("val", stringStream.Storage());
                        subObject.Add("pos", argPos + argOffset + 1);
                        argPos++;
                        argOffset += stringStream.Storage().size() - 2;
                    }
                }
                if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("%d", argPos);
                    infra::JsonObjectFormatter subObject(arguments.SubObject());
                    infra::StringOutputStream::WithStorage<32> digitStream;
                    for (uint8_t i = 0; (nameToMatch[argPos + argOffset + i] >= '0' && nameToMatch[argPos + argOffset + i] <= '9'); i++)
                        digitStream << nameToMatch[argPos + argOffset + i];
                    subObject.Add("val", digitStream.Storage());
                    subObject.Add("pos", argPos + argOffset);
                    argPos++;
                    argOffset += digitStream.Storage().size() - 2;
                }
            } while (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos);
        }
        infra::JsonArray argumentArray(arrayBuffer);
        return argumentArray;
    }

    StepStorage::Step::Step(const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(infra::JsonArray("[]"))
        , tableHeaders(infra::JsonArray("[]"))
        , stepName(stepName)
    {
        nrSteps++;
    }

    StepStorage::Step::Step(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(matchArguments)
        , tableHeaders(tableHeaders)
        , stepName(stepName)
    {
        nrSteps++;
    }

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
        {}

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
        for  (char& c : string)
        {
            infra::StringOutputStream::WithStorage<1> cToInt;
            cToInt << c;
            if (c == '\'' || (c >= '0' && c<= '9'))
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
                    idStream << this->stepStorage.MatchStep(nameToMatchString)->Id();
                    subObject.Add("id", idStream.Storage());
                    subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), this->stepStorage.MatchStep(nameToMatchString)->MatchArguments()) });
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
            if (!MatchArguments(invokeArguments))
            {
                FailureMessage(responseBuffer, "Invalid Arguments", "Exception.Arguments.Invalid");
            }
            else
            {
                SuccessMessage(responseBuffer);
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

    StepStorage::StepStorage() : nrStepMatches(0) {}

    infra::Optional<StepStorage::Step> StepStorage::MatchStep(const infra::BoundedString& nameToMatch)
    {
        nrStepMatches = 0;
        infra::Optional<StepStorage::Step> returnStep;
        for (auto& step : stepList)
        {
            if (MatchStepName(step, nameToMatch))
            {
                returnStep.Emplace(step);
                nrStepMatches++;
                if (step.ContainsArguments())
                    step.SetMatchArguments(step.ParseArguments(nameToMatch, step.MatchArgumentsBuffer()));
            }
        }

        if (nrStepMatches >= 2 || nrStepMatches == 0)
            return infra::none;
        else
            return returnStep;
    }

    bool StepStorage::MatchStepName(StepStorage::Step& step, const infra::BoundedString& nameToMatch)
    {
        uint8_t count = 0;
        uint8_t sizeOffset = 0;
        for (infra::BoundedString::iterator c = nameToMatch.begin(); c != nameToMatch.end(); ++c, count++)
        {
            if (step.StepName()[count] != *c)
            {
                if (((count - 1) >= 0 && (count + 2) <= step.StepName().size()) && (step.StepName()[count - 1] == '\'' && step.StepName()[count] == '%' && step.StepName()[count + 1] == 's' && step.StepName()[count + 2] == '\'' && *(c - 1) == '\''))
                {
                    count += 2;
                    do
                    {
                        sizeOffset++;
                        c++;
                    } while (c != nameToMatch.end() && *c != '\'');
                    sizeOffset -= 2;
                }
                else if ((count + 1 <= step.StepName().size()) && (step.StepName()[count] == '%' && step.StepName()[count + 1] == 'd'))
                {
                    count++;
                    while (c != nameToMatch.end() && *(c + 1) >= '0' && *(c + 1) <= '9')
                    {
                        sizeOffset++;
                        c++;
                    }
                    sizeOffset--;
                }
                else
                    return false;
            }
        }
        if (step.StepName().size() + sizeOffset == nameToMatch.size())
        {
            return true;
        }
        else
            return false;
    }

    infra::Optional<StepStorage::Step> StepStorage::MatchStep(uint8_t id)
    {
        for (auto& step : stepList)
            if (step.Id() == id)
            {
                infra::Optional<StepStorage::Step> tempStep(infra::inPlace, step);
                return tempStep;
            }
        return infra::none;
    }

    void StepStorage::AddStep(const StepStorage::Step& step)
    {
        this->stepList.push_front(step);
    }

    CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer, StepStorage& stepStorage)
        : receiveBufferVector(infra::ReinterpretCastMemoryRange<infra::StaticStorage<uint8_t>>(receiveBuffer))
        , receiveBuffer(receiveBuffer)
        , cucumberParser(stepStorage)
    {}

    void CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        cucumberParser.FormatResponse(stream);

        receiveBufferVector.clear();
        writer = nullptr;
    }

    void CucumberWireProtocolConnectionObserver::DataReceived()
    {
        auto reader = Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        do
        {
            dataBuffer = stream.ContiguousRange();
            receiveBufferVector.insert(receiveBufferVector.end(), dataBuffer.begin(), dataBuffer.end());
        } while (dataBuffer.size() != 0);

        cucumberParser.ParseRequest(receiveBufferVector.range());

        Subject().AckReceived();
        Subject().RequestSendStream(ConnectionObserver::Subject().MaxSendStreamSize());
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(const infra::ByteRange receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, StepStorage& stepStorage)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , stepStorage(stepStorage)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            value.Emplace(this->receiveBuffer, this->stepStorage);
        })
    {}
}
