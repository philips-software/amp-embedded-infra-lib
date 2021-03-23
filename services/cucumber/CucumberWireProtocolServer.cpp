#include "services/cucumber/CucumberWireProtocolServer.hpp"

namespace services
{
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
