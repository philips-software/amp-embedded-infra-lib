#include "services/network_win/EventDispatcherWithNetwork.hpp"
#include <ws2tcpip.h>

namespace services
{
    EventDispatcherWithNetwork::EventDispatcherWithNetwork()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            std::abort();
    }

    EventDispatcherWithNetwork::~EventDispatcherWithNetwork()
    {
        BOOL result = WSACloseEvent(wakeUpEvent);
        assert(result == TRUE);
        WSACleanup();
    }

    void EventDispatcherWithNetwork::RegisterConnection(const infra::SharedPtr<ConnectionWin>& connection)
    {
        connections.push_back(connection);
    }

    void EventDispatcherWithNetwork::RegisterListener(ListenerWin& listener)
    {
        listeners.push_back(listener);
    }

    void EventDispatcherWithNetwork::DeregisterListener(ListenerWin& listener)
    {
        listeners.erase(listener);
    }

    void EventDispatcherWithNetwork::DeregisterConnector(ConnectorWin& connector)
    {
        for (auto c = connectors.begin(); c != connectors.end(); ++c)
            if (&*c == &connector)
            {
                connectors.erase(c);
                return;
            }

        std::abort();
    }

    void EventDispatcherWithNetwork::RegisterDatagram(const infra::SharedPtr<DatagramWin>& datagram)
    {
        datagrams.push_back(datagram);
    }

    void EventDispatcherWithNetwork::RegisterDatagramMultiple(const infra::SharedPtr<DatagramExchangeMultiple>& datagram)
    {
        datagramsMultiple.push_back(datagram);
    }

    infra::SharedPtr<void> EventDispatcherWithNetwork::Listen(uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        assert(versions != IPVersions::ipv6);
        return infra::MakeSharedOnHeap<ListenerWin>(*this, port, factory);
    }

    void EventDispatcherWithNetwork::Connect(ClientConnectionObserverFactory& factory)
    {
        assert(factory.Address().Is<IPv4Address>());
        connectors.emplace_back(*this, factory);
    }

    void EventDispatcherWithNetwork::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        for (auto c = connectors.begin(); c != connectors.end(); ++c)
            if (&c->factory == &factory)
            {
                connectors.erase(c);
                return;
            }

        std::abort();
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions)
    {
        assert(versions != IPVersions::ipv6);
        auto result = infra::MakeSharedOnHeap<DatagramWin>(port, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Listen(DatagramExchangeObserver& observer, IPVersions versions)
    {
        assert(versions != IPVersions::ipv6);
        auto result = infra::MakeSharedOnHeap<DatagramWin>(observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Connect(DatagramExchangeObserver& observer, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramWin>(remote, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramWin>(localPort, remote, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Listen(DatagramExchangeObserver& observer, IPAddress localAddress, uint16_t port, IPVersions versions)
    {
        auto result = infra::MakeSharedOnHeap<DatagramWin>(localAddress, port, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Listen(DatagramExchangeObserver& observer, IPAddress localAddress, IPVersions versions)
    {
        auto result = infra::MakeSharedOnHeap<DatagramWin>(localAddress, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Connect(DatagramExchangeObserver& observer, IPAddress localAddress, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramWin>(localAddress, remote, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Connect(DatagramExchangeObserver& observer, UdpSocket local, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramWin>(local, remote, observer);
        RegisterDatagram(result);
        return result;
    }

    void EventDispatcherWithNetwork::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        auto datagram = std::find(datagrams.begin(), datagrams.end(), datagramExchange);

        if (datagram != datagrams.end())
            datagram->lock()->JoinMulticastGroup(multicastAddress);
        else
        {
            auto datagram = std::find(datagramsMultiple.begin(), datagramsMultiple.end(), datagramExchange);

            if (datagram != datagramsMultiple.end())
                datagram->lock()->JoinMulticastGroup(multicastAddress);
        }
    }

    void EventDispatcherWithNetwork::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        auto datagram = std::find(datagrams.begin(), datagrams.end(), datagramExchange);

        if (datagram != datagrams.end())
            datagram->lock()->LeaveMulticastGroup(multicastAddress);
        else
        {
            auto datagram = std::find(datagramsMultiple.begin(), datagramsMultiple.end(), datagramExchange);

            if (datagram != datagramsMultiple.end())
                datagram->lock()->LeaveMulticastGroup(multicastAddress);
        }
    }

    void EventDispatcherWithNetwork::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress)
    {
        std::abort();
    }

    void EventDispatcherWithNetwork::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv6Address multicastAddress)
    {
        std::abort();
    }

    void EventDispatcherWithNetwork::RequestExecution()
    {
        BOOL result = WSASetEvent(wakeUpEvent);
        assert(result == TRUE);
    }

    void EventDispatcherWithNetwork::Idle()
    {
        std::vector<WSAEVENT> events;
        std::vector<std::function<void()>> functions;

        events.push_back(wakeUpEvent);
        functions.push_back([this]()
        {
            BOOL result = WSAResetEvent(wakeUpEvent);
            assert(result == TRUE);
        });

        for (auto& listener : listeners)
        {
            events.push_back(listener.event);
            functions.push_back([&listener]()
            {
                WSANETWORKEVENTS networkEvents;
                WSAEnumNetworkEvents(listener.listenSocket, listener.event, &networkEvents);
                assert((networkEvents.lNetworkEvents & FD_ACCEPT) != 0);

                listener.Accept();
            });
        }

        for (auto& connector : connectors)
        {
            events.push_back(connector.event);
            functions.push_back([&connector]()
            {
                WSANETWORKEVENTS networkEvents;
                WSAEnumNetworkEvents(connector.connectSocket, connector.event, &networkEvents);
                assert((networkEvents.lNetworkEvents & FD_CONNECT) != 0);

                if (networkEvents.iErrorCode[FD_CONNECT_BIT] != 0)
                    connector.Failed();
                else
                    connector.Connected();
            });
        }

        for (auto& weakConnection : connections)
            if (infra::SharedPtr<ConnectionWin> connection = weakConnection)
                connection->TrySend();

        for (auto& weakConnection : connections)
        {
            if (infra::SharedPtr<ConnectionWin> connection = weakConnection)
                if (connection->socket != 0)
                {
                    connection->UpdateEventFlags();
                    events.push_back(connection->event);
                    functions.push_back([weakConnection]()
                    {
                        if (infra::SharedPtr<ConnectionWin> connection = weakConnection)
                        {
                            WSANETWORKEVENTS networkEvents;
                            WSAEnumNetworkEvents(connection->socket, connection->event, &networkEvents);

                            if ((networkEvents.lNetworkEvents & FD_READ) != 0)
                                connection->Receive();
                            if ((networkEvents.lNetworkEvents & FD_WRITE) != 0)
                                connection->Send();
                            if ((networkEvents.lNetworkEvents & FD_CLOSE) != 0)
                                connection->Receive();
                        }
                    });
                }
        }

        for (auto& weakDatagram : datagrams)
            if (infra::SharedPtr<DatagramWin> datagram = weakDatagram)
                datagram->TrySend();

        for (auto& weakDatagram : datagrams)
        {
            if (infra::SharedPtr<DatagramWin> datagram = weakDatagram)
            {
                datagram->UpdateEventFlags();
                events.push_back(datagram->event);
                functions.push_back([weakDatagram]()
                {
                    if (infra::SharedPtr<DatagramWin> datagram = weakDatagram)
                    {
                        WSANETWORKEVENTS networkEvents;
                        WSAEnumNetworkEvents(datagram->socket, datagram->event, &networkEvents);

                        if ((networkEvents.lNetworkEvents & FD_READ) != 0)
                            datagram->Receive();
                        if ((networkEvents.lNetworkEvents & FD_WRITE) != 0)
                            datagram->Send();
                    }
                });
            }
        }

        DWORD index = WSAWaitForMultipleEvents(events.size(), events.data(), FALSE, WSA_INFINITE, FALSE);
        if (index >= WSA_WAIT_EVENT_0 && index < WSA_WAIT_EVENT_0 + events.size())
            functions[index - WSA_WAIT_EVENT_0]();
        else if (index == WSA_WAIT_FAILED)
        {
            volatile auto error = WSAGetLastError();
            std::abort();
        }
        else
            std::abort();

        connections.remove_if([](const infra::WeakPtr<ConnectionWin>& connection) { return connection.lock() == nullptr; });
        datagrams.remove_if([](const infra::WeakPtr<DatagramWin>& datagram) { return datagram.lock() == nullptr; });
        datagramsMultiple.remove_if([](const infra::WeakPtr<DatagramExchangeMultiple>& datagram) { return datagram.lock() == nullptr; });
    }
}
