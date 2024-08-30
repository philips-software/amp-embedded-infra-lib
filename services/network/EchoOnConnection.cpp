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
            auto readerPtr = reader.Emplace(ConnectionObserver::Subject().ReceiveStream(), *this);
            EchoOnStreams::DataReceived(infra::MakeContainedSharedObject(readerPtr->limitedReader, readerPtr));
        }
        else
            delayReceived = true;
    }

    void EchoOnConnection::RequestSendStream(std::size_t size)
    {
        ConnectionObserver::Subject().RequestSendStream(std::min(size, ConnectionObserver::Subject().MaxSendStreamSize()));
    }

    EchoOnConnection::LimitedReader::LimitedReader(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, EchoOnConnection& connection)
        : reader(std::move(reader))
        , limitedReader(*this->reader, this->reader->Available())
        , connection(connection)
    {}

    EchoOnConnection::LimitedReader::~LimitedReader()
    {
        if (connection.IsAttached())
            connection.ConnectionObserver::Subject().AckReceived();
    }
}
