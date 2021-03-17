#include "services/network/CucumberWireProtocolServer.hpp"

namespace services
{
    uint8_t  StepStorage::Step::nrSteps = 0;

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

    infra::JsonArray StepStorage::Step::InvokeArguments()
    {
        return this->invokeArguments;
    }

    void StepStorage::Step::SetInvokeArguments(infra::JsonArray arguments)
    {
        this->invokeArguments = arguments;
    }

    infra::BoundedString StepStorage::Step::StepName()
    {
        return stepName;
    }

    void StepStorage::Step::StepName(infra::BoundedString stepName)
    {
        this->stepName = stepName;
    }

    bool StepStorage::Step::ContainsArguments()
    {
        if (this->StepName().find("\'%s\'") != infra::BoundedString::npos || this->StepName().find("%d") != infra::BoundedString::npos)
            return true;
        return false;
    }

    infra::JsonArray StepStorage::Step::ParseArguments(const infra::BoundedString& nameToMatch, infra::BoundedString& arrayBuffer)
    {
        {
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, arrayBuffer);
            uint8_t argPos = 0;
            uint8_t argOffset = 0;
            do
            {
                if (this->StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                {
                    argPos = this->StepName().find("\'%s\'", argPos);
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
                if (this->StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = this->StepName().find("%d", argPos);
                    infra::JsonObjectFormatter subObject(arguments.SubObject());
                    infra::StringOutputStream::WithStorage<32> digitStream;
                    for (uint8_t i = 0; (nameToMatch[argPos + argOffset + i] >= '0' && nameToMatch[argPos + argOffset + i] <= '9'); i++)
                        digitStream << nameToMatch[argPos + argOffset + i];
                    subObject.Add("val", digitStream.Storage());
                    subObject.Add("pos", argPos + argOffset);
                    argPos++;
                    argOffset += digitStream.Storage().size() - 2;
                }
            } while (this->StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || this->StepName().find("%d", argPos) != infra::BoundedString::npos);
        }
        infra::JsonArray argumentArray(arrayBuffer);
        return argumentArray;
    }

    StepStorage::Step::Step(const infra::JsonArray& matchArguments, const infra::JsonArray& invokeArguments, const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(matchArguments)
        , invokeArguments(invokeArguments)
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

            infra::BoundedString::WithStorage<64> nameToMatchString;
            nameToMatch.GetString("name_to_match").ToString(nameToMatchString);

            if (MatchName(nameToMatchString))
                matchResult = true;
            else
                matchResult = false;
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

    infra::Optional<infra::BoundedString> CucumberWireProtocolParser::ReformatStepName(const infra::BoundedString& nameToMatch, infra::JsonArray& arguments, infra::BoundedString& bufferString)
    {
        infra::JsonArrayIterator iterator(arguments.begin());

        bufferString.AssignFromStorage(nameToMatch);
        uint8_t argumentOffset = 0;

        for (infra::JsonArrayIterator iterator(arguments.begin()); iterator != arguments.end(); iterator++)
        {
            infra::JsonObject object = iterator->Get<infra::JsonObject>();
            infra::BoundedString::WithStorage<16> val;
            iterator->Get<infra::JsonObject>().GetString("val").ToString(val);
            if ((iterator->Get<infra::JsonObject>().GetInteger("pos") - argumentOffset) <= bufferString.size() && (iterator->Get<infra::JsonObject>().GetInteger("pos") + iterator->Get<infra::JsonObject>().GetString("val").size() - argumentOffset <= bufferString.size()))
            {
                if (val.find_first_not_of("0123456789") == infra::BoundedString::npos)
                    bufferString.replace(bufferString.begin() + iterator->Get<infra::JsonObject>().GetInteger("pos") - argumentOffset, bufferString.begin() + iterator->Get<infra::JsonObject>().GetInteger("pos") + iterator->Get<infra::JsonObject>().GetString("val").size() - argumentOffset, "%d");
                else
                    bufferString.replace(bufferString.begin() + iterator->Get<infra::JsonObject>().GetInteger("pos") - argumentOffset, bufferString.begin() + iterator->Get<infra::JsonObject>().GetInteger("pos") + iterator->Get<infra::JsonObject>().GetString("val").size() - argumentOffset, "%s");
                argumentOffset += iterator->Get<infra::JsonObject>().GetString("val").size() - 2;
            }
            else
                return infra::none;
        }
        infra::Optional<infra::BoundedString> returnString(infra::inPlace, bufferString);
        return returnString;
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

    infra::JsonArray CucumberWireProtocolParser::ParseArguments(infra::BoundedString& inputString)
    {
        {
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, invokeArgumentBuffer);
            uint8_t originalPos = 0;
            uint8_t editPos = 0;

            for(infra::BoundedString::iterator c = inputString.begin(); c != inputString.end(); ++c, originalPos++, editPos++)
            {
                if (*c == '\'')
                {
                    infra::JsonObjectFormatter subObject(arguments.SubObject());
                    infra::StringOutputStream::WithStorage<32> stringStream;
                    for (uint8_t i = 1; inputString[editPos + i] != '\''; i++)
                        stringStream << inputString[editPos + i];
                    subObject.Add("val", stringStream.Storage());
                    inputString.replace(inputString.begin() + editPos + 1, inputString.begin() + editPos + 1 + stringStream.Storage().size(), "%s");
                    subObject.Add("pos", originalPos + 1);
                    c += sizeof("%s");
                    editPos += sizeof("%s");
                    originalPos += stringStream.Storage().size() + 1;
                }
                if ((*c >= '0' && *c <= '9'))
                {
                    infra::JsonObjectFormatter subObject(arguments.SubObject());
                    infra::StringOutputStream::WithStorage<32> digitStream;
                    for (uint8_t i = 0; (inputString[editPos + i] >= '0' && inputString[editPos + i] <= '9'); i++)
                        digitStream << inputString[editPos + i];
                    subObject.Add("val", digitStream.Storage());
                    inputString.replace(inputString.begin() + editPos, inputString.begin() + editPos + digitStream.Storage().size(), "%d");
                    subObject.Add("pos", originalPos);
                    originalPos += digitStream.Storage().size() - 1;
                    c++;
                    editPos++;
                }
            }
        }
        infra::JsonArray argumentArray(invokeArgumentBuffer);
        return argumentArray;
    }

    void CucumberWireProtocolParser::FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream)
    {
        infra::BoundedString::WithStorage<256> responseBuffer;

        if (this->requestType == step_matches)
        {
            if (matchResult == true)
            {
                {
                    infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);

                    result.Add((infra::BoundedConstString) "success");
                    infra::JsonArrayFormatter subArray(result.SubArray());

                    infra::JsonObjectFormatter subObject(subArray.SubObject());

                    infra::BoundedString::WithStorage<64> nameToMatchString;
                    nameToMatch.GetString("name_to_match").ToString(nameToMatchString);

                    infra::StringOutputStream::WithStorage<6> idStream;
                    idStream << this->stepStorage.MatchStep(nameToMatchString)->Id();
                    subObject.Add("id", idStream.Storage());
                    subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), this->stepStorage.MatchStep(nameToMatchString)->MatchArguments()) });
                }
            }
            if (matchResult == false)
                FailureMessage(responseBuffer, "Step not Matched", "Exception.Step.NotFound");
            else
                responseBuffer.insert(responseBuffer.size(), "\n");
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

    bool CucumberWireProtocolParser::MatchName(const infra::BoundedString& nameToMatchString)
    {
        if (this->stepStorage.MatchStep(nameToMatchString) != infra::none)
        {
            return true;
        }
        return false;
    }

    bool CucumberWireProtocolParser::MatchArguments(infra::JsonArray& arguments)
    {
        if (this->stepStorage.MatchStep(invokeId) != infra::none)
        {
            infra::JsonArrayIterator argumentIterator(arguments.begin());
            if (arguments.begin() != arguments.end())
            {
                infra::JsonArrayIterator argumentIterator(arguments.begin());
                argumentIterator = argumentIterator->Get<infra::JsonArray>().begin();

                infra::JsonArray argumentList = argumentIterator->Get<infra::JsonArray>();
                argumentIterator = argumentList.begin();

                infra::JsonArray stepArgumentsList(this->stepStorage.MatchStep(invokeId)->InvokeArguments());
                infra::JsonArrayIterator stepArgumentIterator(stepArgumentsList.begin());

                do
                {
                    if (argumentIterator->Get<infra::JsonString>() != stepArgumentIterator->Get<infra::JsonString>())
                    {
                        return false;
                    }

                    argumentIterator++;
                    stepArgumentIterator++;
                } while (argumentIterator != argumentList.end() && stepArgumentIterator != stepArgumentsList.end());
            }
            return true;
        }
        return false;
    }

    StepStorage::StepStorage(){}

    infra::Optional<StepStorage::Step> StepStorage::MatchStep(const infra::BoundedString& nameToMatch)
    {
        for (auto& step : stepList)
            if (step.StepName() == nameToMatch)
            {
                infra::Optional<StepStorage::Step> tempStep(infra::inPlace, step);
                return tempStep;
            }
        return infra::none;
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
