#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryDecorator.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "gmock/gmock.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
    using Client = services::ClientConnectionObserverFactoryWithNameResolver;
    using ClientMock = testing::StrictMock<services::ClientConnectionObserverFactoryWithNameResolverMock>;
    using CreatedObserver = infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>;

    class ConnectionEnds
    {
    public:
        testing::StrictMock<services::ConnectionMock> lowerConnection;
        infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> observerStorage;
        infra::SharedPtr<services::ConnectionObserverFullMock> observer{ observerStorage.Emplace() };
    };

    class HooksMock
    {
    public:
        MOCK_METHOD(void, LowerConnectFailed, (Client::ConnectFailReason reason));
        MOCK_METHOD(void, AttemptRemoved, ());
        MOCK_METHOD(void, ClientObserverGiven, (Client & requestingClient, bool clientObserverGiven));
    };

    class DerivedDecorated
        : public services::ConnectionWithHostnameDecorator
    {
    public:
        explicit DerivedDecorated(std::size_t& dataReceivedCount)
            : dataReceivedCount(dataReceivedCount)
        {}

        void DataReceived() override
        {
            ++dataReceivedCount;
            services::ConnectionWithHostnameDecorator::DataReceived();
        }

    private:
        std::size_t& dataReceivedCount;
    };

    template<class Decorated>
    class FactoryAccess
        : public services::ConnectionFactoryDecorator<Decorated>
    {
    public:
        using Base = services::ConnectionFactoryDecorator<Decorated>;
        using Connector = typename Base::Connector;

        FactoryAccess(infra::BoundedList<Connector>& connectors, services::ConnectionFactoryWithNameResolver& lower, HooksMock& hooks)
            : Base(connectors, lower)
            , hooks(hooks)
        {}

        using Base::Attempts;
        using Base::CancelAtLower;
        using Base::CancelAttempt;
        using Base::WaitingClients;

    protected:
        HooksMock& hooks;
    };

    class PlainFactory
        : public FactoryAccess<services::ConnectionWithHostnameDecorator>
    {
    public:
        using FactoryAccess::FactoryAccess;

        infra::SharedPtr<services::ConnectionWithHostnameDecorator> CreateConnection(infra::NotifyingSharedOptional<services::ConnectionWithHostnameDecorator>& storage) override
        {
            ++createdConnections;
            return storage.Emplace();
        }

        std::size_t createdConnections = 0;
    };

    class HookedFactory
        : public PlainFactory
    {
    public:
        using PlainFactory::PlainFactory;

        void LowerConnectFailed(Client::ConnectFailReason reason) override
        {
            hooks.LowerConnectFailed(reason);
        }

        void AttemptRemoved() override
        {
            hooks.AttemptRemoved();
        }
    };

    class ObserverGivenFactory
        : public PlainFactory
    {
    public:
        using PlainFactory::PlainFactory;

        void ClientObserverGiven(Connector& connector, const infra::SharedPtr<services::ConnectionObserver>& clientObserver) override
        {
            hooks.ClientObserverGiven(connector.RequestingClient(), clientObserver != nullptr);

            if (clientObserver == nullptr)
                connector.Decline();
            else
                PlainFactory::ClientObserverGiven(connector, clientObserver);
        }
    };

    class DerivedTypeFactory
        : public FactoryAccess<DerivedDecorated>
    {
    public:
        using FactoryAccess::FactoryAccess;

        infra::SharedPtr<DerivedDecorated> CreateConnection(infra::NotifyingSharedOptional<DerivedDecorated>& storage) override
        {
            return storage.Emplace(dataReceivedCount);
        }

        std::size_t dataReceivedCount = 0;
    };

    template<class Factory, std::size_t maxAttempts>
    class ConnectionFactoryDecoratorTestBase
        : public testing::Test
    {
    public:
        ConnectionFactoryDecoratorTestBase()
            : factory(lower, hooks)
        {
            EXPECT_CALL(clientA, Hostname()).WillRepeatedly(testing::Return(hostnameA));
            EXPECT_CALL(clientA, Port()).WillRepeatedly(testing::Return(portA));
            EXPECT_CALL(clientB, Hostname()).WillRepeatedly(testing::Return(hostnameB));
            EXPECT_CALL(clientB, Port()).WillRepeatedly(testing::Return(portB));
            EXPECT_CALL(clientC, Hostname()).WillRepeatedly(testing::Return(hostnameC));
            EXPECT_CALL(clientC, Port()).WillRepeatedly(testing::Return(portC));
            EXPECT_CALL(clientD, Hostname()).WillRepeatedly(testing::Return(hostnameD));
            EXPECT_CALL(clientD, Port()).WillRepeatedly(testing::Return(portD));
        }

        void ExpectLowerConnect(uint16_t port)
        {
            EXPECT_CALL(lower, Connect(testing::_)).WillOnce(testing::Invoke([this, port](Client& connector)
                {
                    EXPECT_THAT(connector.Port(), testing::Eq(port));
                    forwarded.push_back(&connector);
                }));
        }

        void ExpectLowerConnectThatFails(uint16_t port, Client::ConnectFailReason reason)
        {
            EXPECT_CALL(lower, Connect(testing::_)).WillOnce(testing::Invoke([this, port, reason](Client& connector)
                {
                    EXPECT_THAT(connector.Port(), testing::Eq(port));
                    forwarded.push_back(&connector);
                    connector.ConnectionFailed(reason);
                }));
        }

        void ExpectLowerCancel(uint16_t port)
        {
            EXPECT_CALL(lower, CancelConnect(testing::_)).WillOnce(testing::Invoke([port](Client& connector)
                {
                    EXPECT_THAT(connector.Port(), testing::Eq(port));
                }));
        }

        void ExpectClientAttaches(ClientMock& client, ConnectionEnds& ends)
        {
            EXPECT_CALL(client, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this, &ends](CreatedObserver&& createdObserver)
                {
                    AttachClientObserver(createdObserver, ends);
                }));
        }

        void ExpectClientDeclines(ClientMock& client)
        {
            EXPECT_CALL(lowerObserverCreated, Call(false));
            EXPECT_CALL(client, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([](CreatedObserver&& createdObserver)
                {
                    createdObserver(nullptr);
                }));
        }

        void ExpectClientEstablishedLater(ClientMock& client, CreatedObserver& heldCreatedObserver)
        {
            EXPECT_CALL(client, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([&heldCreatedObserver](CreatedObserver&& createdObserver)
                {
                    heldCreatedObserver = std::move(createdObserver);
                }));
        }

        void AttachClientObserver(CreatedObserver& createdObserver, ConnectionEnds& ends)
        {
            testing::Sequence sequence;
            EXPECT_CALL(lowerObserverCreated, Call(true)).InSequence(sequence);
            EXPECT_CALL(*ends.observer, Attached()).InSequence(sequence);
            createdObserver(ends.observer);
        }

        void EstablishLowerConnection(std::size_t forwardedIndex, ConnectionEnds& ends)
        {
            forwarded.at(forwardedIndex)->ConnectionEstablished([this, &ends](infra::SharedPtr<services::ConnectionObserver> trackedConnection)
                {
                    lowerObserverCreated.Call(trackedConnection != nullptr);

                    if (trackedConnection != nullptr)
                        ends.lowerConnection.Attach(trackedConnection);
                });
        }

        void EstablishLowerConnectionWithoutAttaching(std::size_t forwardedIndex)
        {
            forwarded.at(forwardedIndex)->ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> trackedConnection)
                {
                    lowerObserverCreated.Call(trackedConnection != nullptr);
                });
        }

        void EstablishConnection(ClientMock& client, uint16_t port, ConnectionEnds& ends)
        {
            ExpectLowerConnect(port);
            factory.Connect(client);
            ExpectClientAttaches(client, ends);
            EstablishLowerConnection(forwarded.size() - 1, ends);
        }

        void ReleaseConnection(ConnectionEnds& ends)
        {
            EXPECT_CALL(*ends.observer, Detaching());
            ends.lowerConnection.Detach();
        }

        void ExpectAbort(ConnectionEnds& ends)
        {
            EXPECT_CALL(*ends.observer, Abort()).WillOnce(testing::Invoke([&ends]()
                {
                    ends.observer->Subject().AbortAndDestroy();
                }));
            EXPECT_CALL(ends.lowerConnection, AbortAndDestroy());
        }

        void ExpectAbortThatReleases(ConnectionEnds& ends)
        {
            EXPECT_CALL(*ends.observer, Abort()).WillOnce(testing::Invoke([&ends]()
                {
                    ends.observer->Subject().AbortAndDestroy();
                }));
            EXPECT_CALL(*ends.observer, Detaching());
            EXPECT_CALL(ends.lowerConnection, AbortAndDestroy()).WillOnce(testing::Invoke([&ends]()
                {
                    ends.lowerConnection.Detach();
                }));
        }

        const infra::BoundedConstString hostnameA{ "a.example.com" };
        const infra::BoundedConstString hostnameB{ "b.example.com" };
        const infra::BoundedConstString hostnameC{ "c.example.com" };
        const infra::BoundedConstString hostnameD{ "d.example.com" };
        const uint16_t portA = 1;
        const uint16_t portB = 2;
        const uint16_t portC = 3;
        const uint16_t portD = 4;

        testing::StrictMock<services::ConnectionFactoryWithNameResolverMock> lower;
        testing::StrictMock<HooksMock> hooks;
        testing::StrictMock<testing::MockFunction<void(bool createdObserverIsNotNull)>> lowerObserverCreated;
        ClientMock clientA;
        ClientMock clientB;
        ClientMock clientC;
        ClientMock clientD;
        ConnectionEnds connectionA;
        ConnectionEnds connectionB;
        ConnectionEnds connectionC;
        std::vector<Client*> forwarded;
        infra::WithStorage<Factory, typename infra::BoundedList<typename Factory::Connector>::template WithMaxSize<maxAttempts>> factory;
    };

    class ConnectionFactoryDecoratorTest
        : public ConnectionFactoryDecoratorTestBase<PlainFactory, 2>
    {};

    class ConnectionFactoryDecoratorSingleAttemptTest
        : public ConnectionFactoryDecoratorTestBase<PlainFactory, 1>
    {};

    class ConnectionFactoryDecoratorHooksTest
        : public ConnectionFactoryDecoratorTestBase<HookedFactory, 2>
    {};

    class ConnectionFactoryDecoratorObserverGivenTest
        : public ConnectionFactoryDecoratorTestBase<ObserverGivenFactory, 1>
    {};

    class ConnectionFactoryDecoratorDerivedTypeTest
        : public ConnectionFactoryDecoratorTestBase<DerivedTypeFactory, 1>
    {};
}

TEST_F(ConnectionFactoryDecoratorTest, connect_with_free_capacity_forwards_hostname_and_port_at_once)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);

    ASSERT_THAT(forwarded, testing::SizeIs(1));
    EXPECT_THAT(forwarded.at(0)->Hostname(), testing::Eq(hostnameA));
    EXPECT_THAT(forwarded.at(0)->Port(), testing::Eq(portA));
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, connect_when_full_queues_and_forwards_after_release_in_order)
{
    EstablishConnection(clientA, portA, connectionA);
    factory.Connect(clientB);
    factory.Connect(clientC);
    ASSERT_THAT(forwarded, testing::SizeIs(1));

    ExpectLowerConnect(portB);
    ReleaseConnection(connectionA);
    EXPECT_THAT(factory.WaitingClients().has_element(clientC), testing::IsTrue());

    ExpectClientAttaches(clientB, connectionB);
    EstablishLowerConnection(1, connectionB);

    ExpectLowerConnect(portC);
    ReleaseConnection(connectionB);
    EXPECT_THAT(forwarded, testing::SizeIs(3));
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, client_connecting_in_failure_callback_joins_behind_waiting_client)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);

    testing::InSequence sequence;
    EXPECT_CALL(clientA, ConnectionFailed(Client::refused)).WillOnce(testing::Invoke([this](Client::ConnectFailReason)
        {
            factory.Connect(clientA);
        }));
    ExpectLowerConnect(portB);
    forwarded.at(0)->ConnectionFailed(Client::refused);
    EXPECT_THAT(forwarded, testing::SizeIs(2));

    ExpectLowerCancel(portB);
    ExpectLowerConnect(portA);
    factory.CancelConnect(clientB);
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, client_connecting_in_failure_callback_without_waiting_client_is_forwarded_at_once)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);

    std::size_t forwardedInCallback = 0;
    testing::InSequence sequence;
    EXPECT_CALL(clientA, ConnectionFailed(Client::refused)).WillOnce(testing::Invoke([this, &forwardedInCallback](Client::ConnectFailReason)
        {
            factory.Connect(clientA);
            forwardedInCallback = forwarded.size();
        }));
    ExpectLowerConnect(portA);
    forwarded.at(0)->ConnectionFailed(Client::refused);

    EXPECT_THAT(forwardedInCallback, testing::Eq(2));
    EXPECT_THAT(forwarded, testing::SizeIs(2));
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, synchronous_lower_failure_on_forward_forwards_next_waiting_client)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);
    factory.Connect(clientC);

    testing::InSequence sequence;
    EXPECT_CALL(clientA, ConnectionFailed(Client::refused));
    ExpectLowerConnectThatFails(portB, Client::refused);
    EXPECT_CALL(clientB, ConnectionFailed(Client::refused));
    ExpectLowerConnect(portC);
    forwarded.at(0)->ConnectionFailed(Client::refused);

    EXPECT_THAT(forwarded, testing::SizeIs(3));
    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
    EXPECT_THAT(factory.WaitingClients().empty(), testing::IsTrue());
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, cancel_of_waiting_client_does_not_call_lower_and_is_not_forwarded_later)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);

    EXPECT_THAT(factory.CancelAttempt(clientB), testing::IsTrue());
    EXPECT_THAT(factory.WaitingClients().empty(), testing::IsTrue());

    EXPECT_CALL(clientA, ConnectionFailed(Client::refused));
    forwarded.at(0)->ConnectionFailed(Client::refused);

    EXPECT_THAT(forwarded, testing::SizeIs(1));
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, cancel_of_connecting_client_cancels_lower_frees_capacity_and_forwards_next)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);

    testing::InSequence sequence;
    ExpectLowerCancel(portA);
    ExpectLowerConnect(portB);
    factory.CancelConnect(clientA);

    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
}

TEST_F(ConnectionFactoryDecoratorTest, cancel_of_established_client_has_no_effect_also_when_repeated)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    CreatedObserver createdObserver;
    ExpectClientEstablishedLater(clientA, createdObserver);
    EstablishLowerConnection(0, connectionA);

    EXPECT_THAT(factory.CancelAttempt(clientA), testing::IsTrue());
    EXPECT_THAT(factory.CancelAttempt(clientA), testing::IsTrue());

    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
    AttachClientObserver(createdObserver, connectionA);
    ReleaseConnection(connectionA);
}

TEST_F(ConnectionFactoryDecoratorTest, cancel_prefers_second_connecting_attempt_over_established_one)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    CreatedObserver createdObserver;
    ExpectClientEstablishedLater(clientA, createdObserver);
    EstablishLowerConnection(0, connectionA);
    ExpectLowerConnect(portA);
    factory.Connect(clientA);

    EXPECT_CALL(lower, CancelConnect(testing::Ref(*forwarded.at(1))));
    factory.CancelConnect(clientA);

    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
    AttachClientObserver(createdObserver, connectionA);
    ReleaseConnection(connectionA);
}

TEST_F(ConnectionFactoryDecoratorTest, cancel_attempt_of_unknown_client_returns_false_and_cancel_connect_ignores_it)
{
    EXPECT_THAT(factory.CancelAttempt(clientA), testing::IsFalse());
    factory.CancelConnect(clientA);

    ExpectLowerConnect(portB);
    factory.Connect(clientB);
    ExpectLowerCancel(portB);
    EXPECT_THAT(factory.CancelAttempt(clientB), testing::IsTrue());
}

TEST_F(ConnectionFactoryDecoratorObserverGivenTest, declining_in_client_observer_given_override_gives_lower_null_observer_creates_no_connection_and_forwards_next_after_release)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);
    ExpectClientDeclines(clientA);
    EXPECT_CALL(hooks, ClientObserverGiven(testing::Ref(clientA), false));

    ExpectLowerConnect(portB);
    EstablishLowerConnection(0, connectionA);

    EXPECT_THAT(factory.createdConnections, testing::Eq(0));
    EXPECT_THAT(forwarded, testing::SizeIs(2));
}

TEST_F(ConnectionFactoryDecoratorTest, observer_creates_connection_once_attaches_it_to_lower_then_attaches_client_observer)
{
    EstablishConnection(clientA, portA, connectionA);

    EXPECT_THAT(factory.createdConnections, testing::Eq(1));
    EXPECT_THAT(connectionA.lowerConnection.IsAttached(), testing::IsTrue());
    EXPECT_THAT(connectionA.observer->IsAttached(), testing::IsTrue());

    EXPECT_CALL(*connectionA.observer, DataReceived());
    connectionA.lowerConnection.Observer().DataReceived();
    ReleaseConnection(connectionA);
}

TEST_F(ConnectionFactoryDecoratorTest, client_observer_is_not_attached_when_lower_did_not_attach_connection)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    EXPECT_CALL(lowerObserverCreated, Call(true));
    EXPECT_CALL(clientA, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this](CreatedObserver&& createdObserver)
        {
            createdObserver(connectionA.observer);
        }));

    EstablishLowerConnectionWithoutAttaching(0);

    EXPECT_THAT(connectionA.observer->IsAttached(), testing::IsFalse());
    EXPECT_THAT(factory.Attempts().empty(), testing::IsTrue());
}

TEST_F(ConnectionFactoryDecoratorTest, abort_before_observer_aborts_connection_at_once)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    CreatedObserver createdObserver;
    ExpectClientEstablishedLater(clientA, createdObserver);
    EstablishLowerConnection(0, connectionA);

    factory.Attempts().front().Abort();
    EXPECT_THAT(factory.Attempts().front().IsAborted(), testing::IsTrue());

    testing::InSequence sequence;
    EXPECT_CALL(lowerObserverCreated, Call(true));
    EXPECT_CALL(*connectionA.observer, Attached());
    ExpectAbort(connectionA);
    createdObserver(connectionA.observer);

    ReleaseConnection(connectionA);
}

TEST_F(ConnectionFactoryDecoratorTest, abort_after_observer_aborts_connection)
{
    EstablishConnection(clientA, portA, connectionA);
    EXPECT_THAT(factory.Attempts().front().IsAborted(), testing::IsFalse());

    ExpectAbort(connectionA);
    factory.Attempts().front().Abort();

    EXPECT_THAT(factory.Attempts().front().IsAborted(), testing::IsTrue());
    ReleaseConnection(connectionA);
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, capacity_is_kept_until_callback_dropped_and_connection_released_in_either_order)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    CreatedObserver droppedObserver;
    ExpectClientEstablishedLater(clientA, droppedObserver);
    EstablishLowerConnection(0, connectionA);
    factory.Connect(clientB);
    EXPECT_THAT(forwarded, testing::SizeIs(1));

    ExpectLowerConnect(portB);
    droppedObserver = nullptr;

    ExpectClientAttaches(clientB, connectionB);
    EstablishLowerConnection(1, connectionB);
    factory.Connect(clientC);
    EXPECT_THAT(forwarded, testing::SizeIs(2));

    ExpectLowerConnect(portC);
    ReleaseConnection(connectionB);

    CreatedObserver abortedObserver;
    ExpectClientEstablishedLater(clientC, abortedObserver);
    EstablishLowerConnection(2, connectionC);
    factory.Connect(clientD);
    factory.Attempts().front().Abort();
    EXPECT_THAT(forwarded, testing::SizeIs(3));

    EXPECT_CALL(lowerObserverCreated, Call(true));
    EXPECT_CALL(*connectionC.observer, Attached());
    EXPECT_CALL(*connectionC.observer, Abort()).WillOnce(testing::Invoke([this]()
        {
            connectionC.observer->Subject().AbortAndDestroy();
        }));
    EXPECT_CALL(*connectionC.observer, Detaching());
    EXPECT_CALL(connectionC.lowerConnection, AbortAndDestroy()).WillOnce(testing::Invoke([this]()
        {
            connectionC.lowerConnection.Detach();
            EXPECT_THAT(forwarded, testing::SizeIs(3));
        }));
    ExpectLowerConnect(portD);
    abortedObserver(connectionC.observer);

    EXPECT_THAT(forwarded, testing::SizeIs(4));
    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, abort_that_releases_synchronously_removes_attempt_once)
{
    EstablishConnection(clientA, portA, connectionA);
    factory.Connect(clientB);

    ExpectAbortThatReleases(connectionA);
    ExpectLowerConnect(portB);
    factory.Attempts().front().Abort();

    EXPECT_THAT(forwarded, testing::SizeIs(2));
    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
    EXPECT_THAT(factory.WaitingClients().empty(), testing::IsTrue());
}

TEST_F(ConnectionFactoryDecoratorHooksTest, lower_failure_hook_runs_after_removal_and_before_client_is_told)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);

    testing::InSequence sequence;
    EXPECT_CALL(hooks, LowerConnectFailed(Client::refused)).WillOnce(testing::Invoke([this](Client::ConnectFailReason)
        {
            EXPECT_THAT(factory.Attempts().empty(), testing::IsTrue());
        }));
    EXPECT_CALL(clientA, ConnectionFailed(Client::refused));
    EXPECT_CALL(hooks, AttemptRemoved());
    forwarded.at(0)->ConnectionFailed(Client::refused);
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, default_follow_up_forwards_waiting_clients_after_failure_and_after_release)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);
    factory.Connect(clientC);

    {
        testing::InSequence sequence;
        EXPECT_CALL(clientA, ConnectionFailed(Client::refused));
        ExpectLowerConnect(portB);
        forwarded.at(0)->ConnectionFailed(Client::refused);
    }

    ExpectClientAttaches(clientB, connectionB);
    EstablishLowerConnection(1, connectionB);

    ExpectLowerConnect(portC);
    ReleaseConnection(connectionB);
    EXPECT_THAT(forwarded, testing::SizeIs(3));
}

TEST_F(ConnectionFactoryDecoratorHooksTest, overridden_follow_up_runs_after_release_and_failure_but_not_after_cancel)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    ExpectLowerConnect(portB);
    factory.Connect(clientB);
    factory.Connect(clientC);

    ExpectLowerCancel(portA);
    ExpectLowerConnect(portC);
    factory.CancelConnect(clientA);

    ExpectClientAttaches(clientB, connectionB);
    EstablishLowerConnection(1, connectionB);

    EXPECT_CALL(hooks, AttemptRemoved());
    ReleaseConnection(connectionB);

    EXPECT_CALL(hooks, LowerConnectFailed(Client::refused));
    EXPECT_CALL(clientC, ConnectionFailed(Client::refused));
    EXPECT_CALL(hooks, AttemptRemoved());
    forwarded.at(2)->ConnectionFailed(Client::refused);
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, cancel_at_lower_removes_waiting_attempt_without_forwarding)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);

    ExpectLowerCancel(portA);
    factory.CancelAtLower(factory.Attempts().front());

    EXPECT_THAT(factory.Attempts().empty(), testing::IsTrue());
    EXPECT_THAT(factory.WaitingClients().has_element(clientB), testing::IsTrue());
}

TEST_F(ConnectionFactoryDecoratorSingleAttemptTest, derived_class_reads_attempts_and_waiting_clients)
{
    EXPECT_THAT(factory.Attempts().empty(), testing::IsTrue());
    EXPECT_THAT(factory.WaitingClients().empty(), testing::IsTrue());

    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    factory.Connect(clientB);

    EXPECT_THAT(factory.Attempts().size(), testing::Eq(1));
    EXPECT_THAT(factory.Attempts().front().RequestingClient(), testing::Ref(clientA));
    EXPECT_THAT(factory.Attempts().front().Serves(clientA), testing::IsTrue());
    EXPECT_THAT(factory.Attempts().front().Serves(clientB), testing::IsFalse());
    EXPECT_THAT(factory.Attempts().front().IsWaiting(), testing::IsTrue());
    EXPECT_THAT(factory.WaitingClients().has_element(clientB), testing::IsTrue());
}

TEST_F(ConnectionFactoryDecoratorDerivedTypeTest, non_default_decorated_type_is_created_by_derived_class_and_attached)
{
    EstablishConnection(clientA, portA, connectionA);

    EXPECT_CALL(*connectionA.observer, DataReceived());
    connectionA.lowerConnection.Observer().DataReceived();

    EXPECT_THAT(factory.dataReceivedCount, testing::Eq(1));
    ReleaseConnection(connectionA);
}

TEST_F(ConnectionFactoryDecoratorObserverGivenTest, client_observer_given_override_delegating_to_base_attaches_non_null_observer)
{
    ExpectLowerConnect(portA);
    factory.Connect(clientA);
    EXPECT_CALL(hooks, ClientObserverGiven(testing::Ref(clientA), true));
    ExpectClientAttaches(clientA, connectionA);

    EstablishLowerConnection(0, connectionA);

    EXPECT_THAT(factory.createdConnections, testing::Eq(1));
    EXPECT_THAT(connectionA.observer->IsAttached(), testing::IsTrue());
    ReleaseConnection(connectionA);
}
