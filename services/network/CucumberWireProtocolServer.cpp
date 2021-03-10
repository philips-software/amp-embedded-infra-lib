#include "services/network/CucumberWireProtocolServer.hpp"

namespace services
{
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
            bool stepMatchResult = true;
            {
                infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);

                if (stepMatchResult == true)
                {
                    result.Add((infra::BoundedConstString) "success");
                    infra::JsonArrayFormatter subArray(result.SubArray());

                    infra::JsonObjectFormatter subObject(subArray.SubObject());
                    infra::StringOutputStream::WithStorage<6> idStream;
                    idStream << this->matchedId;
                    subObject.Add("id", idStream.Storage());

                    subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), matchedArguments) });

                }
            }
            responseBuffer.insert(responseBuffer.size(), "\n");
        }
        else if (this->requestType == begin_scenario)
            SuccessMessage(responseBuffer);
        else if (this->requestType == end_scenario)
            SuccessMessage(responseBuffer);
        else if (this->requestType == invoke)
        {
            {
                infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, responseBuffer);
                result.Add("pending");
                result.Add("I'll do it later");
            }
            responseBuffer.insert(responseBuffer.size(), "\n");
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
            FailureMessage(responseBuffer, "Invalid Request", "Some.Foreign.ExceptionType");

        stream << infra::StringAsByteRange(responseBuffer);
    }

    CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer)
        : receiveBufferVector(infra::ReinterpretCastMemoryRange<infra::StaticStorage<uint8_t>>(receiveBuffer))
        , receiveBuffer(receiveBuffer)
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

    CucumberWireProtocolServer::CucumberWireProtocolServer(const infra::ByteRange receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            value.Emplace(this->receiveBuffer);
        })
    {}
}
