#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/BoundedString.hpp"
#include "services/network/Connection.hpp"
#include "services/network/SingleConnectionListener.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/IntrusiveForwardList.hpp"

namespace services
{
    class CucumberWireProtocolParser
    {
    public:
        CucumberWireProtocolParser();

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
        bool MatchName(uint8_t& id, infra::JsonArray& arguments);
        void FormatResponse(infra::DataOutputStream::WithErrorPolicy& stream);
        bool MatchArguments(infra::JsonArray& arguments);

        void SuccessMessage(infra::BoundedString& responseBuffer);
        void FailureMessage(infra::BoundedString& responseBuffer, infra::BoundedConstString failMessage, infra::BoundedConstString exceptionType);

    private:
        RequestType requestType;
        infra::JsonObject nameToMatch;
        bool matchResult;
        uint8_t matchedId;
        infra::JsonArray matchedArguments;

        uint8_t invokeId;
        infra::JsonArray invokeArguments;

    public:
        
    };

    class StepNames
    {
    public:
        class Step
            : public infra::IntrusiveList<Step>::NodeType
        {
        public:
            Step(uint8_t id, infra::JsonArray matchArguments, infra::JsonArray invokeArguments, infra::BoundedString stepName);

            uint8_t id;
            infra::JsonArray matchArguments;
            infra::JsonArray invokeArguments;
            infra::BoundedString::WithStorage<128> stepName;

            //void Invoke();
            //bool Match();
        };

    public:
        StepNames();

    public:
        infra::Optional<StepNames::Step> MatchStep(uint8_t id);
        infra::Optional<StepNames::Step> MatchStep(infra::BoundedString nameToMatch);
        //void AddStep();

    private:
        infra::IntrusiveList<Step> stepList;
        StepNames::Step AWiFiNodeIsAvailable;
        StepNames::Step TheConnectivityNodeConnectsToThatNetwork;
        StepNames::Step TheConnectivityNodeShouldBeConnected;
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
