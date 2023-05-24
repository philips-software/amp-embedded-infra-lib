#ifndef SERVICES_ECHO_ON_CONNECTION_HPP
#define SERVICES_ECHO_ON_CONNECTION_HPP

#include "protobuf/echo/Echo.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class EchoOnConnection
        : public EchoOnStreams
        , public ConnectionObserver
    {
    public:
        using EchoOnStreams::EchoOnStreams;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    protected:
        virtual void RequestSendStream(std::size_t size) override;
        virtual void BusyServiceDone() override;
    };
}

#endif
