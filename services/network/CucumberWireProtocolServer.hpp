#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "services/network/Connection.hpp"
#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    class CucumberWireProtocolConnectionObserver
        : public services::ConnectionObserver
    {
    public:
        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
    };

    class CucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        CucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port);

    private:
        infra::Creator<services::ConnectionObserver, CucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

#endif
