#ifndef SERVICES_DATAGRAM_BSD_HPP
#define SERVICES_DATAGRAM_BSD_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/SharedObjectAllocator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Datagram.hpp"
#include <list>
#include <netinet/in.h>
#include <sys/socket.h>

namespace services
{
    class DatagramBsd
        : public services::DatagramExchange
        , public infra::EnableSharedFromThis<DatagramBsd>
    {
    public:
        DatagramBsd(uint16_t port, DatagramExchangeObserver& observer, IPVersions versions = IPVersions::ipv4);
        explicit DatagramBsd(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::ipv4);
        DatagramBsd(const UdpSocket& remote, DatagramExchangeObserver& observer);
        DatagramBsd(uint16_t localPort, const UdpSocket& remote, DatagramExchangeObserver& observer);
        DatagramBsd(IPAddress localAddress, DatagramExchangeObserver& observer);
        DatagramBsd(IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer);
        DatagramBsd(IPAddress localAddress, const UdpSocket& remote, DatagramExchangeObserver& observer);
        DatagramBsd(const UdpSocket& local, const UdpSocket& remote, DatagramExchangeObserver& observer);
        ~DatagramBsd();

        bool SendBufferEmpty() const;

        void RequestSendStream(std::size_t sendSize) override;
        void RequestSendStream(std::size_t sendSize, UdpSocket to) override;

        void Receive(int socketToReceive);
        void Send();
        void TrySend();

        void JoinMulticastGroup(IPv4Address multicastAddress);
        void LeaveMulticastGroup(IPv4Address multicastAddress);
        void JoinMulticastGroup(IPv6Address multicastAddress);
        void LeaveMulticastGroup(IPv6Address multicastAddress);

    private:
        void InitSocket();
        void BindLocal(const UdpSocket& local);
        void BindRemote(const UdpSocket& remote);
        void TryAllocateSendStream();
        int SocketFor(const UdpSocket& to) const;
        int Ipv6Socket() const;

    private:
        class StreamWriterBsd
            : public infra::ByteOutputStreamWriter
        {
        public:
            explicit StreamWriterBsd(DatagramBsd& connection);
            ~StreamWriterBsd();

        private:
            DatagramBsd& connection;
        };

    private:
        friend class EventDispatcherWithNetwork;

        int family = AF_INET;
        bool dualStack = false;
        int socket = -1;
        int socketAdditional = -1;
        IPv4Address localAddress{};
        std::optional<UdpSocket> connectedTo;

        std::optional<infra::BoundedVector<uint8_t>::WithMaxSize<508>> sendBuffer;

        infra::SharedOptional<StreamWriterBsd> streamWriter;
        std::size_t requestedSendSize = 0;
        UdpSocket requestedTo;
        bool trySend = false;
        infra::SharedPtr<DatagramBsd> self;
    };

    using AllocatorDatagramBsd = infra::SharedObjectAllocator<DatagramBsd, void(int)>;
}

#endif
