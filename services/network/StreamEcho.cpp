#include "services/network/StreamEcho.hpp"

namespace services
{
    void StreamEchoConnection::SendInitial()
    {}

    void StreamEchoConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        {
            infra::DataOutputStream::WithErrorPolicy stream(*writer);
            auto receiveStreamReader = Subject().ReceiveStream();
            infra::DataInputStream::WithErrorPolicy receiveStream(*receiveStreamReader);

            while (requestedSize != 0)
            {
                infra::ConstByteRange range = receiveStream.ContiguousRange(requestedSize);
                requestedSize -= range.size();
                stream << range;
            }

            Subject().AckReceived();
            writer = nullptr;
        }

        TryRequestSendStream();
    }

    void StreamEchoConnection::DataReceived()
    {
        if (requestedSize == 0)
            TryRequestSendStream();
    }

    void StreamEchoConnection::TryRequestSendStream()
    {
        auto receiveStreamReader = Subject().ReceiveStream();
        requestedSize = std::min<std::size_t>(receiveStreamReader->Available(), Subject().MaxSendStreamSize());
        if (requestedSize != 0)
            Subject().RequestSendStream(requestedSize);
    }

    StreamEchoServer::StreamEchoServer(AllocatorStreamEchoConnection& allocator, services::ConnectionFactory& listenerFactory, uint16_t port, services::IPVersions versions)
        : listenerFactory(listenerFactory)
        , allocator(allocator)
    {
        listenPort = listenerFactory.Listen(port, *this, versions);
    }

    void StreamEchoServer::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address)
    {
        createdObserver(allocator.Allocate());
    }
}
