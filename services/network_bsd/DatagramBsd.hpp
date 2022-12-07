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
    class EventDispatcherWithNetwork;

    class DatagramBsd
        : public services::DatagramExchange
        , public infra::EnableSharedFromThis<DatagramBsd>
    {
    public:
        DatagramBsd(EventDispatcherWithNetwork& network, uint16_t port, DatagramExchangeObserver& observer);
        DatagramBsd(EventDispatcherWithNetwork& network, DatagramExchangeObserver& observer);
        DatagramBsd(EventDispatcherWithNetwork& network, UdpSocket remote, DatagramExchangeObserver& observer);
        DatagramBsd(EventDispatcherWithNetwork& network, uint16_t localPort, UdpSocket remote, DatagramExchangeObserver& observer);
        ~DatagramBsd();

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual void RequestSendStream(std::size_t sendSize, UdpSocket to) override;

        void Receive();
        void Send();
        void TrySend();

    private:
        void InitSocket();
        void BindLocal(uint16_t port);
        void BindRemote(UdpSocket remote);
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

        EventDispatcherWithNetwork& network;
        int socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        infra::Optional<UdpSocket> connectedTo;

        infra::Optional<infra::BoundedVector<uint8_t>::WithMaxSize<508>> sendBuffer;

        infra::SharedOptional<StreamWriterBsd> streamWriter;
        std::size_t requestedSendSize = 0;
        UdpSocket requestedTo;
        bool trySend = false;
    };

    using AllocatorDatagramBsd = infra::SharedObjectAllocator<DatagramBsd, void(EventDispatcherWithNetwork&, int)>;
}

#endif
