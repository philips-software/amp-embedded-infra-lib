#include "services/network/EchoOnConnection.hpp"
#include "infra/util/SharedPtr.hpp"

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
            keepAliveWhileReading = ConnectionObserver::Subject().ObserverPtr();
            EchoOnStreams::DataReceived(infra::MakeContainedSharedObject(readerPtr->limitedReader, readerPtr));
        }
        else
            delayReceived = true;
    }

    void EchoOnConnection::RequestSendStream(std::size_t size)
    {
        ConnectionObserver::Subject().RequestSendStream(std::min(size, ConnectionObserver::Subject().MaxSendStreamSize()));
    }

    void EchoOnConnection::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        if (forwarder.Allocatable())
        {
            auto forwarderPtr = forwarder.Emplace(Forwarder{ ConnectionObserver::Subject().ObserverPtr(), std::move(reader) });
            auto forwardingReader = infra::MakeContainedSharedObject(*forwarderPtr->reader, forwarderPtr);
            forwarderPtr = nullptr;
            EchoOnStreams::MethodContents(std::move(forwardingReader));
        }
        else
            EchoOnStreams::MethodContents(infra::MakeContainedSharedObject(*forwarder->reader, forwarder.MakePtr()));
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
