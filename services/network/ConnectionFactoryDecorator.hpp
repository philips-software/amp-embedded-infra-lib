#ifndef SERVICES_CONNECTION_FACTORY_DECORATOR_HPP
#define SERVICES_CONNECTION_FACTORY_DECORATOR_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace services
{
    template<class Decorated = ConnectionWithHostnameDecorator, std::enable_if_t<std::is_base_of_v<ConnectionWithHostnameDecorator, Decorated>, std::nullptr_t> = nullptr>
    class ConnectionFactoryDecorator
        : public ConnectionFactoryWithNameResolver
    {
    protected:
        class Connector;

    public:
        using Client = ClientConnectionObserverFactoryWithNameResolver;

        ConnectionFactoryDecorator(infra::BoundedList<Connector>& connectors, ConnectionFactoryWithNameResolver& lower)
            : lower(lower)
            , connectors(connectors)
        {}

        void Connect(Client& client) override
        {
            if (connectors.full() || !waitingClients.empty())
                QueueClient(client);
            else
                StartForward(client);
        }

        void CancelConnect(Client& client) override
        {
            if (waitingClients.has_element(client))
            {
                waitingClients.erase(client);
                return;
            }

            bool known = false;

            for (auto& connector : connectors)
                if (connector.Serves(client))
                {
                    if (connector.IsWaiting())
                    {
                        CancelAtLower(connector);
                        ForwardWaitingClients();
                        return;
                    }

                    known = true;
                }

            really_assert(known);
        }

    protected:
        ~ConnectionFactoryDecorator() = default;

        virtual infra::SharedPtr<Decorated> CreateConnection(infra::NotifyingSharedOptional<Decorated>& storage) = 0;

        virtual void LowerConnectFailed(Client::ConnectFailReason reason)
        {}

        virtual void AttemptRemoved()
        {
            ForwardWaitingClients();
        }

        void QueueClient(Client& client)
        {
            waitingClients.push_back(client);
        }

        void ForwardWaitingClients()
        {
            while (!connectors.full() && !waitingClients.empty())
            {
                auto& client = waitingClients.front();
                waitingClients.pop_front();
                StartForward(client);
            }
        }

        void FailPendingClients(Client::ConnectFailReason reason)
        {
            auto connecting = [](const Connector& connector)
            {
                return connector.IsWaiting();
            };

            for (auto attempt = std::find_if(connectors.begin(), connectors.end(), connecting); attempt != connectors.end(); attempt = std::find_if(connectors.begin(), connectors.end(), connecting))
            {
                auto& failedClient = attempt->RequestingClient();
                CancelAtLower(*attempt);
                failedClient.ConnectionFailed(reason);
            }

            while (!waitingClients.empty())
            {
                auto& failedClient = waitingClients.front();
                waitingClients.pop_front();
                failedClient.ConnectionFailed(reason);
            }
        }

        void AbortEstablishedConnections()
        {
            auto abortable = [](const Connector& connector)
            {
                return !connector.IsWaiting() && !connector.IsAborted();
            };

            aborting = true;

            for (auto attempt = std::find_if(connectors.begin(), connectors.end(), abortable); attempt != connectors.end(); attempt = std::find_if(connectors.begin(), connectors.end(), abortable))
                attempt->Abort();

            aborting = false;
        }

        bool HasAttempts() const
        {
            return !connectors.empty();
        }

        bool HasWaitingClients() const
        {
            return !waitingClients.empty();
        }

        bool HasConnectingAttempts() const
        {
            return std::any_of(connectors.begin(), connectors.end(), [](const Connector& connector)
                {
                    return connector.IsWaiting();
                });
        }

        class Connector
            : public Client
        {
        public:
            Connector(ConnectionFactoryDecorator& factory, Client& client)
                : factory(factory)
                , client(client)
                , connection([this]()
                      {
                          Released();
                      })
                , access([this]()
                      {
                          Released();
                      })
            {}

            infra::BoundedConstString Hostname() const override
            {
                return client.Hostname();
            }

            uint16_t Port() const override
            {
                return client.Port();
            }

            void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override
            {
                established = true;
                attachToLower = std::move(createdObserver);
                auto self = access.MakeShared(*this);
                client.ConnectionEstablished([self](infra::SharedPtr<ConnectionObserver> clientObserver)
                    {
                        self->ObserverGiven(clientObserver);
                    });
            }

            void ConnectionFailed(ConnectFailReason reason) override
            {
                auto& failedClient = client;
                auto& owner = factory;

                owner.connectors.remove(*this);
                owner.LowerConnectFailed(reason);
                failedClient.ConnectionFailed(reason);
                owner.NotifyAttemptRemoved();
            }

        private:
            friend ConnectionFactoryDecorator;

            Client& RequestingClient() const
            {
                return client;
            }

            bool Serves(const Client& candidate) const
            {
                return &candidate == &client;
            }

            bool IsWaiting() const
            {
                return !established;
            }

            bool IsAborted() const
            {
                return aborted;
            }

            void Abort()
            {
                aborted = true;

                if (HasConnection())
                    connection->Abort();
            }

            void ObserverGiven(const infra::SharedPtr<ConnectionObserver>& clientObserver)
            {
                if (clientObserver == nullptr)
                {
                    attachToLower(nullptr);
                    return;
                }

                auto tracked = factory.CreateConnection(connection);
                attachToLower(tracked);

                if (static_cast<ConnectionObserver&>(*tracked).IsAttached())
                    tracked->Attach(clientObserver);

                if (aborted)
                    tracked->Abort();
            }

            bool HasConnection() const
            {
                return !connection.Allocatable();
            }

            void Released()
            {
                if (!access.Referenced() && !HasConnection())
                {
                    auto& owner = factory;
                    owner.connectors.remove(*this);
                    owner.NotifyAttemptRemoved();
                }
            }

        private:
            ConnectionFactoryDecorator& factory;
            Client& client;
            infra::NotifyingSharedOptional<Decorated> connection;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> attachToLower;
            infra::AccessedBySharedPtr access;
            bool established = false;
            bool aborted = false;
        };

    private:
        void CancelAtLower(Connector& connector)
        {
            lower.CancelConnect(connector);
            connectors.remove(connector);
        }

        void StartForward(Client& client)
        {
            connectors.emplace_back(*this, client);
            lower.Connect(connectors.back());
        }

        void NotifyAttemptRemoved()
        {
            if (!aborting)
                AttemptRemoved();
        }

    private:
        ConnectionFactoryWithNameResolver& lower;
        infra::BoundedList<Connector>& connectors;
        infra::IntrusiveList<Client> waitingClients;
        bool aborting = false;
    };
}

#endif
