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
        DatagramBsd(uint16_t port, DatagramExchangeObserver& observer);
        explicit DatagramBsd(DatagramExchangeObserver& observer);
        DatagramBsd(const UdpSocket& remote, DatagramExchangeObserver& observer);
        DatagramBsd(uint16_t localPort, const UdpSocket& remote, DatagramExchangeObserver& observer);
        DatagramBsd(IPAddress localAddress, DatagramExchangeObserver& observer);
        DatagramBsd(IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer);
        DatagramBsd(IPAddress localAddress, const UdpSocket& remote, DatagramExchangeObserver& observer);
        DatagramBsd(const UdpSocket& local, const UdpSocket& remote, DatagramExchangeObserver& observer);
        ~DatagramBsd();

        void RequestSendStream(std::size_t sendSize) override;
        void RequestSendStream(std::size_t sendSize, UdpSocket to) override;

        void Receive();
        void Send();
        void TrySend();

        void JoinMulticastGroup(IPv4Address multicastAddress);
        void LeaveMulticastGroup(IPv4Address multicastAddress);

    private:
        void InitSocket();
        void BindLocal(const UdpSocket& local);
        void BindRemote(const UdpSocket& remote);
        void TryAllocateSendStream();

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

        int socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        IPv4Address localAddress{};
        infra::Optional<UdpSocket> connectedTo;

        infra::Optional<infra::BoundedVector<uint8_t>::WithMaxSize<508>> sendBuffer;

        infra::SharedOptional<StreamWriterBsd> streamWriter;
        std::size_t requestedSendSize = 0;
        UdpSocket requestedTo;
        bool trySend = false;
        infra::SharedPtr<DatagramBsd> self;
    };

    using AllocatorDatagramBsd = infra::SharedObjectAllocator<DatagramBsd, void(int)>;
}

#endif
