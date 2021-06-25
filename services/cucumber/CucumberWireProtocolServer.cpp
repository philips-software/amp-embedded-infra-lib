#include "services/cucumber/CucumberWireProtocolServer.hpp"

namespace services
{
    CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer)
        : receiveBuffer(receiveBuffer)
        , receiveBufferVector(infra::ReinterpretCastMemoryRange<infra::StaticStorage<uint8_t>>(receiveBuffer))
        , controller(*this, scenarioRequestHandler)
        , formatter(parser, controller)
    {}

    void CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        formatter.FormatResponse(stream);

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

        infra::BoundedString input = infra::ByteRangeAsString(receiveBufferVector.range());
        if (parser.Valid(input))
            parser.ParseRequest(input);
        controller.HandleRequest(parser);

        Subject().AckReceived();
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(const infra::ByteRange receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            value.Emplace(this->receiveBuffer);
        })
    {}
}
