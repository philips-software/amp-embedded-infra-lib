#include "services/network_bsd/DatagramBsd.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "services/network_bsd/EventDispatcherWithNetwork.hpp"
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace services
{
    DatagramBsd::DatagramBsd(EventDispatcherWithNetwork& network, uint16_t port, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(port);
    }

    DatagramBsd::DatagramBsd(EventDispatcherWithNetwork& network, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(0);
    }

    DatagramBsd::DatagramBsd(EventDispatcherWithNetwork& network, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(0);
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(EventDispatcherWithNetwork& network, uint16_t localPort, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(localPort);
        BindRemote(remote);
    }

    DatagramBsd::~DatagramBsd()
    {
        GetObserver().Detach();

        int result = close(socket);
        if (result == -1)
            std::abort();
    }

    void DatagramBsd::RequestSendStream(std::size_t sendSize)
    {
        RequestSendStream(sendSize, *connectedTo);
    }

    void DatagramBsd::RequestSendStream(std::size_t sendSize, UdpSocket to)
    {
        assert(streamWriter.Allocatable());
        requestedSendSize = sendSize;
        requestedTo = to;
        TryAllocateSendStream();
    }

    void DatagramBsd::Receive()
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<508> receiveBuffer;
        receiveBuffer.resize(receiveBuffer.max_size());

        sockaddr_in fromAddress{};
        socklen_t fromAddressSize = sizeof(fromAddress);

        auto received = recvfrom(socket, reinterpret_cast<char*>(receiveBuffer.data()), receiveBuffer.size(), 0, reinterpret_cast<sockaddr*>(&fromAddress), &fromAddressSize);
        if (received == -1)
        {
            if (errno != EWOULDBLOCK && errno != EMSGSIZE)
                std::abort();
            return;
        }
        else if (fromAddressSize == sizeof(fromAddress) && fromAddress.sin_family == AF_INET)
        {
            receiveBuffer.resize(received);
            auto reader = infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>();
            reader->Storage() = std::vector<uint8_t>(receiveBuffer.begin(), receiveBuffer.end());

            auto from = Udpv4Socket{ services::ConvertFromUint32(ntohl(fromAddress.sin_addr.s_addr)), fromAddress.sin_port };

            GetObserver().DataReceived(std::move(reader), from);
        }
    }

    void DatagramBsd::Send()
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(requestedTo.Get<Udpv4Socket>().first));
        address.sin_port = htons(requestedTo.Get<Udpv4Socket>().second);
        int sent = sendto(socket, reinterpret_cast<char*>(sendBuffer->data()), sendBuffer->size(), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address));

        if (sent == -1)
        {
            if (errno != EWOULDBLOCK)
                std::abort();
            return;
        }

        sendBuffer = infra::none;
    }

    void DatagramBsd::TrySend()
    {
        if (trySend)
        {
            trySend = false;
            Send();
        }
    }

    void DatagramBsd::InitSocket()
    {
        assert(socket != -1);

        std::array<char, 1> option{ true };
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, option.data(), option.size());

        if (fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK) == -1)
            std::abort();
    }

    void DatagramBsd::BindLocal(uint16_t port)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        auto result = bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        assert(result == 0);
    }

    void DatagramBsd::BindRemote(UdpSocket remote)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(remote.Get<Udpv4Socket>().first));
        address.sin_port = htons(remote.Get<Udpv4Socket>().second);
        auto result = connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        assert(result == 0);

        connectedTo = remote;
    }

    void DatagramBsd::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffer)
        {
            sendBuffer.Emplace(requestedSendSize, 0);
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<DatagramBsd>& object)
                {
                infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object);
                object->GetObserver().SendStreamAvailable(std::move(writer)); },
                SharedFromThis());
        }
    }

    DatagramBsd::StreamWriterBsd::StreamWriterBsd(DatagramBsd& connection)
        : infra::ByteOutputStreamWriter(infra::MakeRange(*connection.sendBuffer))
        , connection(connection)
    {}

    DatagramBsd::StreamWriterBsd::~StreamWriterBsd()
    {
        connection.sendBuffer->resize(Processed().size());
        connection.trySend = true;
    }
}
