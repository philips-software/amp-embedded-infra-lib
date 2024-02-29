#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/SharedPtr.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/ConnectionMbedTls.hpp"
#include "services/network/test_doubles/Certificates.hpp"
#include "services/network/test_doubles/ConnectionLoopBack.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "gmock/gmock.h"

class ConnectionMbedTlsTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    ConnectionMbedTlsTest()
        : thisListener(infra::UnOwnedSharedPtr(*this))
        , connectionFactory(network, serverCertificates, randomDataGenerator)
    {
        serverCertificates.AddCertificateAuthority(services::testCaCertificate);
        serverCertificates.AddOwnCertificate(services::testServerCertificate, services::testServerKey, randomDataGenerator);

        clientCertificates.AddCertificateAuthority(services::testCaCertificate);
        clientCertificates.AddOwnCertificate(services::testClientCertificate, services::testClientKey, randomDataGenerator);
    }

    services::ServerConnectionObserverFactoryMock serverObserverFactory;
    services::ClientConnectionObserverFactoryMock clientObserverFactory;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    testing::StrictMock<services::ConnectionFactoryMock> network;
    services::ConnectionLoopBackFactory loopBackNetwork;
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    infra::SharedPtr<void> thisListener;
    services::CertificatesMbedTls serverCertificates;
    services::CertificatesMbedTls clientCertificates;
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 1, 1> connectionFactory;
};

TEST_F(ConnectionMbedTlsTest, when_allocation_on_network_fails_Listen_returns_nullptr)
{
    EXPECT_CALL(network, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr)));
    infra::SharedPtr<void> listener = connectionFactory.Listen(1234, serverObserverFactory);
    EXPECT_EQ(nullptr, listener);
    EXPECT_CALL(clientObserverFactory, Destructor);
}

TEST_F(ConnectionMbedTlsTest, when_listener_allocation_fails_Listen_returns_nullptr)
{
    EXPECT_CALL(network, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(thisListener)));
    infra::SharedPtr<void> listener1 = connectionFactory.Listen(1234, serverObserverFactory);
    infra::SharedPtr<void> listener2 = connectionFactory.Listen(1234, serverObserverFactory);
    EXPECT_EQ(nullptr, listener2);
    EXPECT_CALL(clientObserverFactory, Destructor);
}

TEST_F(ConnectionMbedTlsTest, Listen_returns_listener)
{
    EXPECT_CALL(network, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(thisListener)));
    infra::SharedPtr<void> listener = connectionFactory.Listen(1234, serverObserverFactory);
    EXPECT_NE(nullptr, listener);
    EXPECT_CALL(clientObserverFactory, Destructor);
}

TEST_F(ConnectionMbedTlsTest, create_connection)
{
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 0, 1> tlsNetworkClient(loopBackNetwork, clientCertificates, randomDataGenerator);
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 1, 0> tlsNetworkServer(loopBackNetwork, serverCertificates, randomDataGenerator);
    infra::SharedPtr<void> listener = tlsNetworkServer.Listen(1234, serverObserverFactory);

    EXPECT_CALL(clientObserverFactory, Port()).WillOnce(testing::Return(1234));
    tlsNetworkClient.Connect(clientObserverFactory);

    infra::SharedOptional<services::ConnectionObserverMock> observer1;
    infra::SharedOptional<services::ConnectionObserverMock> observer2;
    EXPECT_CALL(serverObserverFactory, ConnectionAcceptedMock(testing::_, testing::_))
        .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
            {
                createdObserver(observer1.Emplace());
            }));
    EXPECT_CALL(clientObserverFactory, Address());
    EXPECT_CALL(clientObserverFactory, ConnectionEstablishedMock(testing::_))
        .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
            {
                createdObserver(observer2.Emplace());
            }));
    ExecuteAllActions();
    observer1->Subject().AbortAndDestroy();
    EXPECT_CALL(clientObserverFactory, Destructor);
}

TEST_F(ConnectionMbedTlsTest, send_and_receive_data)
{
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 1, 0> tlsNetworkServer(loopBackNetwork, serverCertificates, randomDataGenerator);
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 0, 1> tlsNetworkClient(loopBackNetwork, clientCertificates, randomDataGenerator);
    infra::SharedPtr<void> listener = tlsNetworkServer.Listen(1234, serverObserverFactory);

    EXPECT_CALL(clientObserverFactory, Port()).WillOnce(testing::Return(1234));
    tlsNetworkClient.Connect(clientObserverFactory);

    infra::SharedOptional<services::ConnectionObserverStub> observer1;
    infra::SharedOptional<services::ConnectionObserverStub> observer2;
    EXPECT_CALL(serverObserverFactory, ConnectionAcceptedMock(testing::_, testing::_))
        .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
            {
                createdObserver(observer1.Emplace());
            }));
    EXPECT_CALL(clientObserverFactory, Address());
    EXPECT_CALL(clientObserverFactory, ConnectionEstablishedMock(testing::_))
        .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
            {
                createdObserver(observer2.Emplace());
            }));
    ExecuteAllActions();

    observer2->SendData(std::vector<uint8_t>{ 1, 2, 3, 4 });
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4 }), observer1->receivedData);

    observer1->SendData(std::vector<uint8_t>{ 5, 6, 7, 8 });
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 5, 6, 7, 8 }), observer2->receivedData);

    observer1->Subject().AbortAndDestroy();
    EXPECT_CALL(clientObserverFactory, Destructor);
}

TEST_F(ConnectionMbedTlsTest, reopen_connection)
{
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 1, 0> tlsNetworkServer(loopBackNetwork, serverCertificates, randomDataGenerator);
    services::ConnectionFactoryMbedTls::WithMaxConnectionsListenersAndConnectors<2, 0, 1> tlsNetworkClient(loopBackNetwork, clientCertificates, randomDataGenerator);
    infra::SharedPtr<void> listener = tlsNetworkServer.Listen(1234, serverObserverFactory);

    {
        EXPECT_CALL(clientObserverFactory, Port()).WillOnce(testing::Return(1234));
        tlsNetworkClient.Connect(clientObserverFactory);

        infra::SharedOptional<services::ConnectionObserverStub> observer1;
        infra::SharedOptional<services::ConnectionObserverStub> observer2;
        EXPECT_CALL(serverObserverFactory, ConnectionAcceptedMock(testing::_, testing::_))
            .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
                {
                    createdObserver(observer1.Emplace());
                }));
        EXPECT_CALL(clientObserverFactory, Address());
        EXPECT_CALL(clientObserverFactory, ConnectionEstablishedMock(testing::_))
            .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
                {
                    createdObserver(observer2.Emplace());
                }));
        ExecuteAllActions();

        observer1->Subject().AbortAndDestroy();
    }

    {
        EXPECT_CALL(clientObserverFactory, Port()).WillOnce(testing::Return(1234));
        tlsNetworkClient.Connect(clientObserverFactory);

        infra::SharedOptional<services::ConnectionObserverStub> observer1;
        infra::SharedOptional<services::ConnectionObserverStub> observer2;
        EXPECT_CALL(serverObserverFactory, ConnectionAcceptedMock(testing::_, testing::_))
            .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
                {
                    createdObserver(observer1.Emplace());
                }));
        EXPECT_CALL(clientObserverFactory, Address());
        EXPECT_CALL(clientObserverFactory, ConnectionEstablishedMock(testing::_))
            .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
                {
                    createdObserver(observer2.Emplace());
                }));
        ExecuteAllActions();

        observer1->Subject().AbortAndDestroy();
    }
    EXPECT_CALL(clientObserverFactory, Destructor);
}

class ConnectionWithNameResolverMbedTlsTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    ConnectionWithNameResolverMbedTlsTest()
    {
        clientCertificates.AddCertificateAuthority(services::testCaCertificate);
        clientCertificates.AddOwnCertificate(services::testClientCertificate, services::testClientKey, randomDataGenerator);
    }

    testing::StrictMock<services::ClientConnectionObserverFactoryWithNameResolverMock> clientObserverFactory;
    testing::StrictMock<services::ConnectionFactoryWithNameResolverMock> network;
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    services::CertificatesMbedTls clientCertificates;
    infra::Optional<services::ConnectionWithHostnameMock> connection;
};

TEST_F(ConnectionWithNameResolverMbedTlsTest, cancel_connection)
{
    services::ConnectionFactoryWithNameResolverMbedTls::WithMaxConnectionsListenersAndConnectors<2, 1> tlsNetworkClient(network, clientCertificates, randomDataGenerator);

    EXPECT_CALL(network, Connect(testing::_));
    tlsNetworkClient.Connect(clientObserverFactory);

    EXPECT_CALL(network, CancelConnect(testing::_));
    tlsNetworkClient.CancelConnect(clientObserverFactory);

    EXPECT_CALL(clientObserverFactory, Destructor);
}

TEST_F(ConnectionWithNameResolverMbedTlsTest, create_connection)
{
    services::ConnectionFactoryWithNameResolverMbedTls::WithMaxConnectionsListenersAndConnectors<2, 1> tlsNetworkClient(network, clientCertificates, randomDataGenerator);
    EXPECT_CALL(clientObserverFactory, Port()).WillOnce(testing::Return(1234));
    EXPECT_CALL(clientObserverFactory, Hostname()).WillRepeatedly(testing::Return("something"));
    infra::SharedOptional<services::ConnectionObserverStub> observer1;

    EXPECT_CALL(network, Connect(testing::_)).WillOnce(testing::Invoke([this](services::ClientConnectionObserverFactoryWithNameResolver& clientObserverFactory)
        {
            EXPECT_EQ("something", clientObserverFactory.Hostname());
            EXPECT_EQ(1234, clientObserverFactory.Port());
            connection.Emplace();
            EXPECT_CALL(*connection, RequestSendStream(0));
            EXPECT_CALL(*connection, MaxSendStreamSize).WillOnce(testing::Return(0));
            clientObserverFactory.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> observer)
                {
                    connection->Attach(observer);
                });
        }));

    EXPECT_CALL(clientObserverFactory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([this, &observer1](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClient)
        {
            createdClient(observer1.Emplace());
        }));

    tlsNetworkClient.Connect(clientObserverFactory);

    ExecuteAllActions();
    EXPECT_CALL(*connection, AbortAndDestroy).WillOnce(testing::Invoke([this]()
        {
            connection = infra::none;
        }));
    observer1->Subject().AbortAndDestroy();
    EXPECT_CALL(clientObserverFactory, Destructor);
}
