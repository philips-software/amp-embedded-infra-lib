#include "services/network_instantiations/EventDispatcherWithNetworkBsd.hpp"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace
{
    void SetNonBlocking(int fileDescriptor)
    {
        if (fcntl(fileDescriptor, F_SETFL, fcntl(fileDescriptor, F_GETFL, 0) | O_NONBLOCK) == -1)
            std::abort();
    }
}

namespace services
{
    EventDispatcherWithNetwork::EventDispatcherWithNetwork()
    {
        if (pipe(wakeUpEvent) == -1)
            std::abort();

        SetNonBlocking(wakeUpEvent[0]);
        SetNonBlocking(wakeUpEvent[1]);
    }

    EventDispatcherWithNetwork::~EventDispatcherWithNetwork()
    {
        close(wakeUpEvent[0]);
        close(wakeUpEvent[1]);
    }

    void EventDispatcherWithNetwork::RegisterConnection(const infra::SharedPtr<ConnectionBsd>& connection)
    {
        connections.push_back(connection);
    }

    void EventDispatcherWithNetwork::RegisterListener(ListenerBsd& listener)
    {
        listeners.push_back(listener);
    }

    void EventDispatcherWithNetwork::DeregisterListener(ListenerBsd& listener)
    {
        listeners.erase(listener);
    }

    void EventDispatcherWithNetwork::DeregisterConnector(ConnectorBsd& connector)
    {
        for (auto c = connectors.begin(); c != connectors.end(); ++c)
            if (&*c == &connector)
            {
                connectors.erase(c);
                return;
            }

        std::abort();
    }

    void EventDispatcherWithNetwork::RegisterDatagram(const infra::SharedPtr<DatagramBsd>& datagram)
    {
        datagrams.push_back(datagram);
    }

    bool EventDispatcherWithNetwork::OpenConnections() const
    {
        return !connectors.empty() || !connections.empty();
    }

    infra::SharedPtr<void> EventDispatcherWithNetwork::Listen(uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        assert(versions != IPVersions::ipv6);
        return infra::MakeSharedOnHeap<ListenerBsd>(*this, port, factory);
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
        auto result = infra::MakeSharedOnHeap<DatagramBsd>(port, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Listen(DatagramExchangeObserver& observer, IPVersions versions)
    {
        assert(versions != IPVersions::ipv6);
        auto result = infra::MakeSharedOnHeap<DatagramBsd>(observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Connect(DatagramExchangeObserver& observer, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramBsd>(remote, observer);
        RegisterDatagram(result);
        return result;
    }

    infra::SharedPtr<DatagramExchange> EventDispatcherWithNetwork::Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramBsd>(localPort, remote, observer);
        RegisterDatagram(result);
        return result;
    }

    void EventDispatcherWithNetwork::JoinMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        auto datagram = std::find(datagrams.begin(), datagrams.end(), datagramExchange);

        if (datagram != datagrams.end())
            datagram->lock()->JoinMulticastGroup(multicastAddress);
    }

    void EventDispatcherWithNetwork::LeaveMulticastGroup(infra::SharedPtr<DatagramExchange> datagramExchange, IPv4Address multicastAddress)
    {
        auto datagram = std::find(datagrams.begin(), datagrams.end(), datagramExchange);

        if (datagram != datagrams.end())
            datagram->lock()->LeaveMulticastGroup(multicastAddress);
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
        if (write(wakeUpEvent[1], "x", 1) == -1 && errno != EAGAIN)
            std::abort();
    }

    void EventDispatcherWithNetwork::Idle()
    {
        FD_ZERO(&readFileDescriptors);
        FD_ZERO(&writeFileDescriptors);
        FD_ZERO(&exceptFileDescriptors);

        AddFileDescriptorToSet(wakeUpEvent[0], readFileDescriptors);

        for (auto& listener : listeners)
            AddFileDescriptorToSet(listener.listenSocket, readFileDescriptors);

        for (auto& connector : connectors)
        {
            AddFileDescriptorToSet(connector.connectSocket, writeFileDescriptors);
            AddFileDescriptorToSet(connector.connectSocket, exceptFileDescriptors);
        }

        for (auto& weakConnection : connections)
            if (infra::SharedPtr<ConnectionBsd> connection = weakConnection)
                connection->TrySend();

        for (auto& weakConnection : connections)
        {
            if (infra::SharedPtr<ConnectionBsd> connection = weakConnection)
            {
                AddFileDescriptorToSet(connection->socket, readFileDescriptors);
                AddFileDescriptorToSet(connection->socket, writeFileDescriptors);
            }
        }

        for (auto& weakDatagram : datagrams)
            if (infra::SharedPtr<DatagramBsd> datagram = weakDatagram)
                datagram->TrySend();

        for (auto& weakDatagram : datagrams)
        {
            if (infra::SharedPtr<DatagramBsd> datagram = weakDatagram)
            {
                AddFileDescriptorToSet(datagram->socket, readFileDescriptors);
                AddFileDescriptorToSet(datagram->socket, writeFileDescriptors);
            }
        }

        int result = 0;
        do
        {
            result = select(numberOfFileDescriptors + 1, &readFileDescriptors, &writeFileDescriptors, &exceptFileDescriptors, nullptr);
        } while (result == -1 && errno == EINTR);

        if (result == -1)
            std::abort();

        while (true)
        {
            char dummy;
            if (read(wakeUpEvent[0], &dummy, 1) == -1)
            {
                if (errno == EAGAIN)
                    break;
                else
                    std::abort();
            }
        }

        for (auto& listener : listeners)
            if (FD_ISSET(listener.listenSocket, &readFileDescriptors))
                listener.Accept();

        for (auto index = connectors.begin(); index != connectors.end();)
        {
            auto& connector = *index;
            ++index;

            if (FD_ISSET(connector.connectSocket, &writeFileDescriptors))
                connector.Connected();
            else if (FD_ISSET(connector.connectSocket, &exceptFileDescriptors))
                connector.Failed();
        }

        for (auto& weakConnection : connections)
        {
            if (infra::SharedPtr<ConnectionBsd> connection = weakConnection)
            {
                if (FD_ISSET(connection->socket, &readFileDescriptors))
                    connection->Receive();
                if (FD_ISSET(connection->socket, &writeFileDescriptors))
                    connection->Send();
            }
        }

        for (auto& weakDatagram : datagrams)
        {
            if (infra::SharedPtr<DatagramBsd> datagram = weakDatagram)
            {
                if (FD_ISSET(datagram->socket, &readFileDescriptors))
                    datagram->Receive();
                if (FD_ISSET(datagram->socket, &writeFileDescriptors))
                    datagram->TrySend();
            }
        }

        connections.remove_if([](const infra::WeakPtr<ConnectionBsd>& connection)
            {
                return connection.lock() == nullptr;
            });
        datagrams.remove_if([](const infra::WeakPtr<DatagramBsd>& datagram)
            {
                return datagram.lock() == nullptr;
            });
    }

    void EventDispatcherWithNetwork::AddFileDescriptorToSet(int fileDescriptor, fd_set& set)
    {
        FD_SET(fileDescriptor, &set);
        numberOfFileDescriptors = std::max(numberOfFileDescriptors, fileDescriptor);
    }
}
