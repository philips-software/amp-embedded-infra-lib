#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/util/BoundedVector.hpp"
#include "services/network/Connection.hpp"
#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    class CucumberWireProtocolParser
    {
        enum RequestType
        {
            step_matches,
            invoke,
            snippet_text,
            begin_scenario,
            end_scenario,
            invalid
        };

    public:
        void ParseRequest(infra::ByteRange inputRange);
        void FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream);

        void SuccessMessage(infra::BoundedString& responseBuffer);
        void FailureMessage(infra::BoundedString& responseBuffer, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

    private:
        RequestType requestType;
        infra::JsonObject nameToMatch;
        uint8_t matchedId;
        infra::JsonArray matchedArguments;

        uint8_t invokeId;
        infra::JsonArray invokeArguments;
    };

    class CucumberWireProtocolConnectionObserver
        : public services::ConnectionObserver
    {
    public:
        CucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    private:
        const infra::ByteRange receiveBuffer;
        infra::BoundedVector<uint8_t> receiveBufferVector;
        infra::ConstByteRange dataBuffer;
        CucumberWireProtocolParser cucumberParser;
    };

    class CucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        template<size_t BufferSize>
        using WithBuffer = infra::WithStorage<CucumberWireProtocolServer, std::array<uint8_t, BufferSize>>;

        CucumberWireProtocolServer(const infra::ByteRange receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port);

    private:
        const infra::ByteRange receiveBuffer;
        infra::Creator<services::ConnectionObserver, CucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

#endif
