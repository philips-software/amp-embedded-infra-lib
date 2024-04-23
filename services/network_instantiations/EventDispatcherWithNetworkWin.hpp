#ifndef SERVICES_EVENT_DISPATCHER_WITH_NETWORK_HPP
#define SERVICES_EVENT_DISPATCHER_WITH_NETWORK_HPP

#include "services/network/Multicast.hpp"
#include "services/network_instantiations/ConnectionWin.hpp"
#include "services/network_instantiations/DatagramWin.hpp"

namespace services
{
    class EventDispatcherWithNetwork
        : public infra::EventDispatcherWithWeakPtr::WithSize<50>
        , public ConnectionFactory
        , public DatagramFactoryWithLocalIpBinding
        , public Multicast
    {
    public:
        EventDispatcherWithNetwork();
        ~EventDispatcherWithNetwork();

        void RegisterConnection(const infra::SharedPtr<ConnectionWin>& connection);
        void RegisterListener(ListenerWin& listener);
        void DeregisterListener(ListenerWin& listener);
        void DeregisterConnector(ConnectorWin& connector);
        void RegisterDatagram(const infra::SharedPtr<DatagramWin>& datagram);
        void RegisterDatagramMultiple(const infra::SharedPtr<DatagramExchangeMultiple>& datagram);

        bool ConnectionsOpen() const;

    public:
        // Implementation of ConnectionFactory
        infra::SharedPtr<void> Listen(uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions) override;
        void Connect(ClientConnectionObserverFactory& factory) override;
        void CancelConnect(ClientConnectionObserverFactory& factory) override;

        // Implementation of DatagramFactoryWithLocalIpBinding
        infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions = IPVersions::both) override;
        infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::both) override;
        infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket remote) override;
        infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote) override;
        infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPAddress localAddress, uint16_t port, IPVersions versions = IPVersions::both) override;
        infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPAddress localAddress, IPVersions versions = IPVersions::both) override;
        infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, IPAddress localAddress, UdpSocket remote) override;
        infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket local, UdpSocket remote) override;

        // Implementation of Multicast
        void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;
        void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;

    protected:
        void RequestExecution() override;
        void Idle() override;

    private:
        std::list<infra::WeakPtr<ConnectionWin>> connections;
        infra::IntrusiveList<ListenerWin> listeners;
        std::list<ConnectorWin> connectors;
        std::list<infra::WeakPtr<DatagramWin>> datagrams;
        std::list<infra::WeakPtr<DatagramExchangeMultiple>> datagramsMultiple;
        WSAEVENT wakeUpEvent = WSACreateEvent();
    };
}

#endif
