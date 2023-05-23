#include "protobuf/echo/EchoOnConnection.hpp"

namespace services
{
    void EchoOnConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        SetStreamWriter(std::move(writer));
    }

    void EchoOnConnection::DataReceived()
    {
        while (!ServiceBusy())
        {
            infra::SharedPtr<infra::StreamReaderWithRewinding> reader = ConnectionObserver::Subject().ReceiveStream();

            if (!ProcessMessage(*reader))
                break;

            if (!ServiceBusy()) // The message was not executed when ServiceBusy() is true, so don't ack the received data
                ConnectionObserver::Subject().AckReceived();
        }
    }

    void EchoOnConnection::RequestSendStream(std::size_t size)
    {
        ConnectionObserver::Subject().RequestSendStream(size);
    }

    void EchoOnConnection::BusyServiceDone()
    {
        DataReceived();
    }
}
