#ifndef SERVICES_EVENT_DISPATCHER_WITH_NETWORK_HPP
#define SERVICES_EVENT_DISPATCHER_WITH_NETWORK_HPP

#include "services/network/Multicast.hpp"
#include "services/network_instantiations/ConnectionBsd.hpp"
#include "services/network_instantiations/DatagramBsd.hpp"
#include <sys/select.h>

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
        ~EventDispatcherWithNetwork() override;

        void RegisterConnection(const infra::SharedPtr<ConnectionBsd>& connection);
        void RegisterListener(ListenerBsd& listener);
        void DeregisterListener(ListenerBsd& listener);
        void DeregisterConnector(ConnectorBsd& connector);
        void RegisterDatagram(const infra::SharedPtr<DatagramBsd>& datagram);

    public:
        // Implementation of ConnectionFactory
        infra::SharedPtr<void> Listen(uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions) override;
        void Connect(ClientConnectionObserverFactory& factory) override;
        void CancelConnect(ClientConnectionObserverFactory& factory) override;

        // Implementation of DatagramFactory
        infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions = IPVersions::both) override;
        infra::SharedPtr<DatagramExchange> Listen(DatagramExchangeObserver& observer, IPVersions versions = IPVersions::both) override;
        infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, UdpSocket remote) override;
        infra::SharedPtr<DatagramExchange> Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote) override;

        // Implementation of Multicast
        void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress) override;
        void JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;
        void LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress) override;

    protected:
        void RequestExecution() override;
        void Idle() override;

    private:
        void AddFileDescriptorToSet(int fileDescriptor, fd_set& set);

    private:
        std::list<infra::WeakPtr<ConnectionBsd>> connections;
        infra::IntrusiveList<ListenerBsd> listeners;
        std::list<ConnectorBsd> connectors;
        std::list<infra::WeakPtr<DatagramBsd>> datagrams;
        int numberOfFileDescriptors = 0;
        int wakeUpEvent[2]{ 0 };
        fd_set readFileDescriptors;
        fd_set writeFileDescriptors;
        fd_set exceptFileDescriptors;
    };
}

#endif
