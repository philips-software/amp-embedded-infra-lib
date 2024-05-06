#include "hal/synchronous_interfaces/test_doubles/SynchronousFixedRandomDataGenerator.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/ProxyCreatorMock.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/HttpPageWebSocket.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/network/WebSocketServerConnectionObserver.hpp"
#include "services/network/test_doubles/ConnectionFactoryWithNameResolverStub.hpp"
#include "services/network/test_doubles/ConnectionLoopBack.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"
#include "services/util/test_doubles/StoppableMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class WebSocketClientObserverFactoryMock
        : public services::WebSocketClientObserverFactory
    {
    public:
        MOCK_CONST_METHOD0(Url, infra::BoundedString());
        MOCK_CONST_METHOD0(Port, uint16_t());
        MOCK_METHOD1(ConnectionEstablished, void(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver));
        MOCK_METHOD1(ConnectionFailed, void(ConnectFailReason reason));
    };
}

class WebSocketIntegrationTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(WebSocketIntegrationTest, create_connection)
{
    services::HttpPageServer httpServer;
    infra::Creator<services::ConnectionObserver, services::WebSocketServerConnectionObserver::WithBufferSizes<512, 512>, void()> webSocketServerConnectionCreator;
    services::WebSocketObserverFactoryImpl websocketObserverFactory({ webSocketServerConnectionCreator });
    services::HttpPageWebSocket webSocketPage("path", websocketObserverFactory);
    httpServer.AddPage(webSocketPage);
    services::HttpServerConnectionObserver::WithBuffer<2048> httpServerConnection(httpServer);
    auto httpServerConnectionPtr = infra::UnOwnedSharedPtr(httpServerConnection);

    services::ServerConnectionObserverFactoryMock serverObserverFactory;
    services::ConnectionLoopBackFactory network;
    services::ConnectionLoopBackListener listener(5, network, serverObserverFactory);

    EXPECT_CALL(serverObserverFactory, ConnectionAccepted(testing::_, testing::_))
        .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
            {
                createdObserver(httpServerConnectionPtr);
            }));

    services::ConnectionFactoryWithNameResolverStub connectionFactoryWithNameResolver(network, services::IPv4AddressLocalHost());
    hal::SynchronousFixedRandomDataGenerator randomDataGenerator({ 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 });
    services::HttpClientConnectorWithNameResolverImpl<> clientConnector(connectionFactoryWithNameResolver);
    infra::Creator<services::Stoppable, services::HttpClientWebSocketInitiation, void(services::WebSocketClientObserverFactory & clientObserverFactory, services::HttpClientWebSocketInitiationResult & result, hal::SynchronousRandomDataGenerator & randomDataGenerator)> httpClientInitiationCreator(
        [&clientConnector](std::optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
            services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        {
            value.emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
        });
    services::WebSocketClientFactorySingleConnection webSocketClientFactory(randomDataGenerator, { httpClientInitiationCreator });

    WebSocketClientObserverFactoryMock clientObserverFactory;
    infra::BoundedString::WithStorage<16> url("hostname/path");
    EXPECT_CALL(clientObserverFactory, Url()).WillRepeatedly(testing::Return(url));
    EXPECT_CALL(clientObserverFactory, Port()).WillRepeatedly(testing::Return(5));
    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> clientConnection;
    EXPECT_CALL(clientObserverFactory, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this, &clientConnection](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClient)
        {
            auto clientConnectionPtr = clientConnection.emplace();
            EXPECT_CALL(*clientConnection, Attached());
            createdClient(clientConnectionPtr);
        }));
    webSocketClientFactory.Connect(clientObserverFactory);
    ExecuteAllActions();

    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> serverConnection;
    auto serverConnectionPtr = serverConnection.emplace();
    EXPECT_CALL(*serverConnection, Attached());
    webSocketServerConnectionCreator->Attach(serverConnectionPtr);

    // Send data from client to server
    EXPECT_CALL(*clientConnection, SendStreamAvailable(testing::_)).WillOnce(testing::Invoke([this, &serverConnection](const infra::SharedPtr<infra::StreamWriter>& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            stream << "Hello!";

            EXPECT_CALL(*serverConnection, DataReceived()).WillOnce(testing::Invoke([this, &serverConnection]()
                {
                    auto reader = serverConnection->Subject().ReceiveStream();
                    infra::TextInputStream::WithErrorPolicy inputStream(*reader);
                    infra::BoundedString::WithStorage<6> receivedText(6);
                    inputStream >> receivedText;
                    EXPECT_EQ("Hello!", receivedText);
                }));
        }));
    clientConnection->Subject().RequestSendStream(512);
    ExecuteAllActions();

    // Send data from server to client
    EXPECT_CALL(*serverConnection, SendStreamAvailable(testing::_)).WillOnce(testing::Invoke([this, &clientConnection](const infra::SharedPtr<infra::StreamWriter>& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            stream << "Howdy!";

            EXPECT_CALL(*clientConnection, DataReceived()).WillOnce(testing::Invoke([this, &clientConnection]()
                {
                    auto reader = clientConnection->Subject().ReceiveStream();
                    infra::TextInputStream::WithErrorPolicy inputStream(*reader);
                    infra::BoundedString::WithStorage<6> receivedText(6);
                    inputStream >> receivedText;
                    EXPECT_EQ("Howdy!", receivedText);
                }));
        }));
    serverConnection->Subject().RequestSendStream(512);
    ExecuteAllActions();

    EXPECT_CALL(*clientConnection, Detaching());
    EXPECT_CALL(*serverConnection, Detaching());
    clientConnection->Subject().AbortAndDestroy();
    ExecuteAllActions();
}

TEST_F(WebSocketIntegrationTest, create_connection_failed)
{
    services::HttpPageServer httpServer;
    services::HttpServerConnectionObserver::WithBuffer<2048> httpServerConnection(httpServer);
    auto httpServerConnectionPtr = infra::UnOwnedSharedPtr(httpServerConnection);

    services::ServerConnectionObserverFactoryMock serverObserverFactory;
    services::ConnectionLoopBackFactory network;
    services::ConnectionLoopBackListener listener(5, network, serverObserverFactory);

    EXPECT_CALL(serverObserverFactory, ConnectionAccepted(testing::_, testing::_))
        .WillOnce(testing::Invoke([&](infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
            {
                createdObserver(httpServerConnectionPtr);
            }));

    services::ConnectionFactoryWithNameResolverStub connectionFactoryWithNameResolver(network, services::IPv4AddressLocalHost());
    hal::SynchronousFixedRandomDataGenerator randomDataGenerator({ 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 });
    services::HttpClientConnectorWithNameResolverImpl<> clientConnector(connectionFactoryWithNameResolver);
    infra::Creator<services::Stoppable, services::HttpClientWebSocketInitiation, void(services::WebSocketClientObserverFactory & clientObserverFactory, services::HttpClientWebSocketInitiationResult & result, hal::SynchronousRandomDataGenerator & randomDataGenerator)> httpClientInitiationCreator(
        [&clientConnector](std::optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
            services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        {
            value.emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
        });
    services::WebSocketClientFactorySingleConnection webSocketClientFactory(randomDataGenerator, { httpClientInitiationCreator });

    WebSocketClientObserverFactoryMock clientObserverFactory;
    infra::BoundedString::WithStorage<16> url("hostname/path");
    EXPECT_CALL(clientObserverFactory, Url()).WillRepeatedly(testing::Return(url));
    EXPECT_CALL(clientObserverFactory, Port()).WillRepeatedly(testing::Return(5));
    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> clientConnection;
    EXPECT_CALL(clientObserverFactory, ConnectionFailed(services::WebSocketClientObserverFactory::ConnectFailReason::upgradeFailed));
    webSocketClientFactory.Connect(clientObserverFactory);
    ExecuteAllActions();
}
