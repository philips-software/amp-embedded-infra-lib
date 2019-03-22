#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason Convert(ClientConnectionObserverFactory::ConnectFailReason reason)
    {
        switch (reason)
        {
            case ClientConnectionObserverFactory::ConnectFailReason::refused:
                return ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::refused;
            case ClientConnectionObserverFactory::ConnectFailReason::connectionAllocationFailed:
                return ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::connectionAllocationFailed;
            default:
                std::abort();
        }
    }

    ConnectionFactoryWithNameResolverImpl::ConnectionFactoryWithNameResolverImpl(infra::BoundedList<Action>& actions, ConnectionFactory& connectionFactory, NameResolver& nameLookup)
        : connectionFactory(connectionFactory)
        , nameLookup(nameLookup)
        , actions(actions)
    {}

    void ConnectionFactoryWithNameResolverImpl::Connect(ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        waitingActions.push_back(factory);
        CheckNameLookup();
    }

    void ConnectionFactoryWithNameResolverImpl::CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        if (waitingActions.has_element(factory))
            waitingActions.erase(factory);
        else
            for (auto& action : actions)
                if (action.Remove(factory))
                    break;
    }

    void ConnectionFactoryWithNameResolverImpl::CheckNameLookup()
    {
        if (!waitingActions.empty() && !actions.full())
        {
            actions.emplace_back(*this, waitingActions.front());
            waitingActions.pop_front();
        }
    }

    void ConnectionFactoryWithNameResolverImpl::ActionIsDone(Action& action)
    {
        actions.remove(action);
        CheckNameLookup();
    }

    ConnectionFactoryWithNameResolverImpl::Action::Action(ConnectionFactoryWithNameResolverImpl& connectionFactory, ClientConnectionObserverFactoryWithNameResolver& clientConnectionFactory)
        : connectionFactory(connectionFactory)
        , clientConnectionFactory(clientConnectionFactory)
    {
        connectionFactory.nameLookup.Lookup(*this);
    }

    bool ConnectionFactoryWithNameResolverImpl::Action::Remove(ClientConnectionObserverFactoryWithNameResolver& clientConnectionFactory)
    {
        if (&clientConnectionFactory == &this->clientConnectionFactory)
        {
            if (connecting)
                connectionFactory.connectionFactory.CancelConnect(*this);
            else
                connectionFactory.nameLookup.CancelLookup(*this);
            return true;
        }
        else
            return false;
    }

    infra::BoundedConstString ConnectionFactoryWithNameResolverImpl::Action::Hostname() const
    {
        return clientConnectionFactory.Hostname();
    }

    void ConnectionFactoryWithNameResolverImpl::Action::NameLookupDone(IPAddress address)
    {
        this->address = address;
        connectionFactory.connectionFactory.Connect(*this);
        connecting = true;
    }

    void ConnectionFactoryWithNameResolverImpl::Action::NameLookupFailed()
    {
        auto& clientConnectionFactory = this->clientConnectionFactory;
        connectionFactory.ActionIsDone(*this);
        clientConnectionFactory.ConnectionFailed(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::nameLookupFailed);
    }

    IPAddress ConnectionFactoryWithNameResolverImpl::Action::Address() const
    {
        return address;
    }

    uint16_t ConnectionFactoryWithNameResolverImpl::Action::Port() const
    {
        return clientConnectionFactory.Port();
    }

    void ConnectionFactoryWithNameResolverImpl::Action::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        auto& clientConnectionFactory = this->clientConnectionFactory;
        connectionFactory.ActionIsDone(*this);
        clientConnectionFactory.ConnectionEstablished(std::move(createdObserver));
    }

    void ConnectionFactoryWithNameResolverImpl::Action::ConnectionFailed(ConnectFailReason reason)
    {
        auto& clientConnectionFactory = this->clientConnectionFactory;
        connectionFactory.ActionIsDone(*this);
        clientConnectionFactory.ConnectionFailed(Convert(reason));
    }
}
