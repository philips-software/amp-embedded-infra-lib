#ifndef SERVICES_DATAGRAM_WIN_HPP
#define SERVICES_DATAGRAM_WIN_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/SharedObjectAllocator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Datagram.hpp"
#include <list>
#include <winsock2.h>

namespace services
{
    class EventDispatcherWithNetwork;

    class DatagramWin
        : public services::DatagramExchange
        , public infra::EnableSharedFromThis<DatagramWin>
    {
    public:
        DatagramWin(EventDispatcherWithNetwork& network, uint16_t port, DatagramExchangeObserver& observer);
        DatagramWin(EventDispatcherWithNetwork& network, DatagramExchangeObserver& observer);
        DatagramWin(EventDispatcherWithNetwork& network, UdpSocket remote, DatagramExchangeObserver& observer);
        DatagramWin(EventDispatcherWithNetwork& network, uint16_t localPort, UdpSocket remote, DatagramExchangeObserver& observer);
        ~DatagramWin();

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual void RequestSendStream(std::size_t sendSize, UdpSocket to) override;
        
        void Receive();
        void Send();
        void TrySend();
        void UpdateEventFlags();

    private:
        void InitSocket();
        void BindLocal(uint16_t port);
        void BindRemote(UdpSocket remote);
        void TryAllocateSendStream();

    private:
        class StreamWriterWin
            : public infra::ByteOutputStreamWriter
        {
        public:
            StreamWriterWin(DatagramWin& connection);
            ~StreamWriterWin();

        private:
            DatagramWin& connection;
        };

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        SOCKET socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        WSAEVENT event = WSACreateEvent();
        infra::Optional<UdpSocket> connectedTo;

        infra::Optional<infra::BoundedVector<uint8_t>::WithMaxSize<508>> sendBuffer;

        infra::SharedOptional<StreamWriterWin> streamWriter;
        std::size_t requestedSendSize = 0;
        UdpSocket requestedTo;
        bool trySend = false;
    };

    using AllocatorDatagramWin = infra::SharedObjectAllocator<DatagramWin, void(EventDispatcherWithNetwork&, SOCKET)>;
}

#endif
