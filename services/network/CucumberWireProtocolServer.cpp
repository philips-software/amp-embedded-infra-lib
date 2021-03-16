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

    StepStorage::Step::Step(infra::JsonArray matchArguments, infra::JsonArray invokeArguments, infra::BoundedString stepName)
        : id(nrSteps)
        , matchArguments(matchArguments)
        , invokeArguments(invokeArguments)
        , stepName(stepName)
    {
        nrSteps++;
    }

    CucumberWireProtocolParser::CucumberWireProtocolParser(StepStorage& stepStorage)
        : matchedArguments(infra::JsonArray("[]"))
        , invokeArguments(infra::JsonArray("[]"))
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

    void CucumberWireProtocolParser::ParseRequest(infra::ByteRange inputRange)
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
            if (MatchName(matchedId, matchedArguments))
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
                    infra::StringOutputStream::WithStorage<6> idStream;
                    idStream << this->matchedId;
                    subObject.Add("id", idStream.Storage());

                    subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), matchedArguments) });
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

    bool CucumberWireProtocolParser::MatchName(uint8_t& id, infra::JsonArray& arguments)
    {
        infra::BoundedString::WithStorage<64> nameToMatchString;
        
        nameToMatch.GetString("name_to_match").ToString(nameToMatchString);
        if (this->stepStorage.MatchStep(nameToMatchString) != infra::none)
        {
            id = stepStorage.MatchStep(nameToMatchString)->Id();
            arguments = stepStorage.MatchStep(nameToMatchString)->MatchArguments();
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

    infra::Optional<StepStorage::Step> StepStorage::MatchStep(const infra::BoundedString nameToMatch)
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

    void StepStorage::AddStep(StepStorage::Step& step)
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
