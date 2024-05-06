#include "services/network/EchoOnConnection.hpp"

namespace services
{
    void EchoOnConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        EchoOnStreams::SendStreamAvailable(std::move(writer));
    }

    void EchoOnConnection::DataReceived()
    {
        if (reader.Allocatable())
        {
            auto readerPtr = reader.emplace(ConnectionObserver::Subject().ReceiveStream());
            EchoOnStreams::DataReceived(infra::MakeContainedSharedObject(readerPtr->limitedReader, readerPtr));
        }
        else
            delayReceived = true;
    }

    void EchoOnConnection::RequestSendStream(std::size_t size)
    {
        ConnectionObserver::Subject().RequestSendStream(std::min(size, ConnectionObserver::Subject().MaxSendStreamSize()));
    }

    void EchoOnConnection::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    EchoOnConnection::LimitedReader::LimitedReader(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        : reader(std::move(reader))
        , limitedReader(*this->reader, this->reader->Available())
    {}
}
