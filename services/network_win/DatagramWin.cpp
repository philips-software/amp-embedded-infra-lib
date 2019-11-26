#include "infra/stream/ByteInputStream.hpp"
#include "services/network_win/DatagramWin.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"

namespace services
{
    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, uint16_t port, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(port);
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(0);
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(0);
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, uint16_t localPort, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(localPort);
        BindRemote(remote);
    }

    DatagramWin::~DatagramWin()
    {
        GetObserver().Detach();

        BOOL result = WSACloseEvent(event);
        assert(result == TRUE);
        result = closesocket(socket);
        if (result == SOCKET_ERROR)
        {
            DWORD error = GetLastError();
            std::abort();
        }
    }

    void DatagramWin::RequestSendStream(std::size_t sendSize)
    {
        RequestSendStream(sendSize, *connectedTo);
    }

    void DatagramWin::RequestSendStream(std::size_t sendSize, UdpSocket to)
    {
        assert(streamWriter.Allocatable());
        requestedSendSize = sendSize;
        requestedTo = to;
        TryAllocateSendStream();
    }

    void DatagramWin::Receive()
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<508> receiveBuffer;
        receiveBuffer.resize(receiveBuffer.max_size());

        sockaddr_in fromAddress{};
        int fromAddressSize = sizeof(fromAddress);

        auto received = recvfrom(socket, reinterpret_cast<char*>(receiveBuffer.data()), receiveBuffer.size(), 0, reinterpret_cast<sockaddr*>(&fromAddress), &fromAddressSize);
        if (received == SOCKET_ERROR)
        {
            auto error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
                std::abort();
            return;
        }
        else if (fromAddressSize == sizeof(fromAddress) && fromAddress.sin_family == AF_INET)
        {
            receiveBuffer.resize(received);
            infra::ByteInputStreamReader reader(infra::MakeRange(receiveBuffer));

            auto from = Udpv4Socket{ services::ConvertFromUint32(htonl(fromAddress.sin_addr.s_addr)), fromAddress.sin_port };

            GetObserver().DataReceived(reader, from);
        }
    }

    void DatagramWin::Send()
    {
        UpdateEventFlags();     // If there is something to send, update the flags before calling send, because FD_SEND is an edge-triggered event.
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(requestedTo.Get<Udpv4Socket>().first));
        address.sin_port = htons(requestedTo.Get<Udpv4Socket>().second);
        int sent = sendto(socket, reinterpret_cast<char*>(sendBuffer->data()), sendBuffer->size(), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address));

        if (sent == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                std::abort();
            return;
        }

        sendBuffer = infra::none;
        UpdateEventFlags();
    }

    void DatagramWin::TrySend()
    {
        if (trySend)
        {
            trySend = false;
            Send();
        }
    }

    void DatagramWin::UpdateEventFlags()
    {
        int result = WSAEventSelect(socket, event, FD_READ | (sendBuffer ? FD_WRITE : 0));
        assert(result == 0);
    }

    void DatagramWin::InitSocket()
    {
        assert(socket != INVALID_SOCKET);

        ULONG nonBlock = 1;
        if (ioctlsocket(socket, FIONBIO, &nonBlock) == SOCKET_ERROR)
            std::abort();

        UpdateEventFlags();
    }

    void DatagramWin::BindLocal(uint16_t port)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        auto result = bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        auto error = WSAGetLastError();
        assert(result == 0);
    }

    void DatagramWin::BindRemote(UdpSocket remote)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(remote.Get<Udpv4Socket>().first));
        address.sin_port = htons(remote.Get<Udpv4Socket>().second);
        auto result = connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        auto error = WSAGetLastError();
        assert(result == 0);

        connectedTo = remote;
    }

    void DatagramWin::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffer)
        {
            sendBuffer.Emplace(requestedSendSize, 0);
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<DatagramWin>& object)
            {
                infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object);
                object->GetObserver().SendStreamAvailable(std::move(writer));
            }, SharedFromThis());
        }
    }

    DatagramWin::StreamWriterWin::StreamWriterWin(DatagramWin& connection)
        : infra::ByteOutputStreamWriter(infra::MakeRange(*connection.sendBuffer))
        , connection(connection)
    {}

    DatagramWin::StreamWriterWin::~StreamWriterWin()
    {
        connection.sendBuffer->resize(Processed().size());
        connection.trySend = true;
    }
}
