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
#include <iphlpapi.h>

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
        DatagramWin(EventDispatcherWithNetwork& network, IPAddress localAddress, DatagramExchangeObserver& observer);
        DatagramWin(EventDispatcherWithNetwork& network, IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer);
        DatagramWin(EventDispatcherWithNetwork& network, IPAddress localAddress, UdpSocket remote, DatagramExchangeObserver& observer);
        DatagramWin(EventDispatcherWithNetwork& network, UdpSocket local, UdpSocket remote, DatagramExchangeObserver& observer);
        ~DatagramWin();

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual void RequestSendStream(std::size_t sendSize, UdpSocket to) override;

        void Receive();
        void Send();
        void TrySend();
        void UpdateEventFlags();

    private:
        void InitSocket();
        void BindLocal(UdpSocket local);
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

    class DatagramFactoryWithLocalIpBinding
        : public DatagramFactory
    {
    public:
        using DatagramFactory::Listen;
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPAddress localAddress, uint16_t port, IPVersions versions = IPVersions::both) = 0;
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPAddress localAddress, IPVersions versions = IPVersions::both) = 0;
        using DatagramFactory::Connect;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, IPAddress localAddress, UdpSocket remote) = 0;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket local, UdpSocket remote) = 0;
    };

    class DatagramExchangeMultiple
        : public DatagramExchange
    {
    public:
        DatagramExchangeMultiple(DatagramExchangeObserver& observer);
        ~DatagramExchangeMultiple();

        void Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, uint16_t port, IPVersions versions);
        void Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, IPVersions versions);
        void Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, UdpSocket remote);
        void Add(DatagramFactoryWithLocalIpBinding& factory, UdpSocket local, UdpSocket remote);

        // Implementation of DatagramExchange
        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual void RequestSendStream(std::size_t sendSize, UdpSocket to) override;

    private:
        class Observer
            : public DatagramExchangeObserver
        {
        public:
            Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, uint16_t port, IPVersions versions);
            Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, IPVersions versions);
            Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, UdpSocket remote);
            Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, UdpSocket local, UdpSocket remote);

            // Implementation of DatagramExchangeObserver
            virtual void DataReceived(infra::StreamReaderWithRewinding& reader, UdpSocket from) override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            DatagramExchangeMultiple& parent;
            infra::SharedPtr<DatagramExchange> exchange;
        };

    private:
        class MultipleWriter
            : public infra::StreamWriter
        {
        public:
            MultipleWriter(const std::vector<infra::SharedPtr<infra::StreamWriter>>& writers);

            virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual std::size_t GetProcessedBytesSince(std::size_t marker) const override;
            virtual infra::ByteRange SaveState(std::size_t marker) override;
            virtual void RestoreState(infra::ByteRange range) override;
            virtual infra::ByteRange Overwrite(std::size_t marker) override;

        private:
            std::vector<infra::SharedPtr<infra::StreamWriter>> writers;
        };

    private:
        std::vector<infra::SharedPtr<Observer>> observers;
        std::vector<infra::SharedPtr<infra::StreamWriter>> writers;

        infra::SharedOptional<MultipleWriter> multipleWriter;
    };

    class UdpOnAllInterfaces
        : public DatagramFactory
    {
    public:
        UdpOnAllInterfaces(DatagramFactoryWithLocalIpBinding& datagramFactory);

        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions = IPVersions::both) override;
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::both) override;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket remote) override;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote) override;

    private:
        std::vector<IPv4Address> GetIpAddresses();
        IPv4Address Translate(const IP_ADDRESS_STRING& address);

    private:
        DatagramFactoryWithLocalIpBinding& datagramFactory;
    };
}

#endif
