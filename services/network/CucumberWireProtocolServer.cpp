#include "services/network/CucumberWireProtocolServer.hpp"

namespace services
{
    void CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        stream << "[\"success\",[{\"id\":\"1\", \"args\":[]}]]\n";

        writer = nullptr;
    }

    void CucumberWireProtocolConnectionObserver::DataReceived()
    {
        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        ConnectionObserver::Subject().AckReceived();
        reader = nullptr;

        ConnectionObserver::Subject().RequestSendStream(ConnectionObserver::Subject().MaxSendStreamSize());
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            value.Emplace();
        })
    {}
}
