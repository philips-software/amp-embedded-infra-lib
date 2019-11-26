#ifndef SERVICES_EVENT_DISPATCHER_WITH_NETWORK_HPP
#define SERVICES_EVENT_DISPATCHER_WITH_NETWORK_HPP

#include "services/network/Multicast.hpp"
#include "services/network_win/ConnectionWin.hpp"
#include "services/network_win/DatagramWin.hpp"

namespace services
{
    class EventDispatcherWithNetwork
        : public infra::EventDispatcherWithWeakPtr::WithSize<50>
        , public ConnectionFactory
        , public DatagramFactory
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

    public:
        // Implementation of ConnectionFactory
        virtual infra::SharedPtr<void> Listen(uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions) override;
        virtual void Connect(ClientConnectionObserverFactory& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& factory) override;

        // Implementation of DatagramFactory
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions = IPVersions::both) override;
        virtual infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::both) override;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket remote) override;
        virtual infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote) override;

        // Implementation of Multicast
        virtual void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        virtual void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        virtual void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;
        virtual void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;

    protected:
        virtual void RequestExecution() override;
        virtual void Idle() override;

    private:
        std::list<infra::WeakPtr<ConnectionWin>> connections;
        infra::IntrusiveList<ListenerWin> listeners;
        std::list<ConnectorWin> connectors;
        std::list<infra::WeakPtr<DatagramWin>> datagrams;
        WSAEVENT wakeUpEvent = WSACreateEvent();
    };
}

#endif
