#include "services/network/EchoOnConnection.hpp"

namespace services
{
    void EchoOnConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        EchoOnStreams::SendStreamAvailable(std::move(writer));
    }

    void EchoOnConnection::DataReceived()
    {
        auto readerPtr = ConnectionObserver::Subject().ReceiveStream();
        limitedReader.Emplace(*readerPtr, readerPtr->Available());
        EchoOnStreams::DataReceived(infra::MakeContainedSharedObject(*limitedReader, readerPtr));
    }

    void EchoOnConnection::RequestSendStream(std::size_t size)
    {
        ConnectionObserver::Subject().RequestSendStream(std::min(size, ConnectionObserver::Subject().MaxSendStreamSize()));
    }

    void EchoOnConnection::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }
}
