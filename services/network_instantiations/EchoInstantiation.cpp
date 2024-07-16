#include "services/network_instantiations/EchoInstantiation.hpp"
#include "infra/timer/Waiting.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"

namespace application
{
    std::shared_ptr<services::Echo> OpenEcho(infra::BoundedString target, services::ConnectionFactoryWithNameResolver& connectionFactory)
    {
        std::shared_ptr<services::Echo> result;

        if (target.substr(0, 3) == "COM" || target.substr(0, 4) == "/dev")
        {
            auto echoClient = std::make_shared<main_::EchoOnUart<256>>(target);
            result = std::shared_ptr<services::Echo>(echoClient, &echoClient->echo);
        }
        else if (services::SchemeFromUrl(target) == "ws")
        {
            if (!infra::host::WaitUntilDone(
                    [&](const std::function<void()>& done)
                    {
                        static hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

                        auto echoClient = std::make_shared<EchoClientWebSocket>(connectionFactory, randomDataGenerator, services::HostFromUrl(target), services::PortFromUrl(target).ValueOr(80));
                        echoClient->OnDone([&, echoClient](services::Echo& echo)
                            {
                                result = std::shared_ptr<services::Echo>(echoClient, &echo);
                                done();
                            });
                    },
                    std::chrono::seconds(10)))
                throw std::runtime_error("Couldn't open websocket connection");
        }
        else if (services::SchemeFromUrl(target) == "tcp")
        {
            if (!infra::host::WaitUntilDone(
                    [&](const std::function<void()>& done)
                    {
                        auto echoClient = std::make_shared<EchoClientTcp>(connectionFactory, services::HostFromUrl(target), services::PortFromUrl(target).ValueOr(1234));
                        echoClient->OnDone([&, echoClient](services::Echo& echo)
                            {
                                result = std::shared_ptr<services::Echo>(echoClient, &echo);
                                done();
                            });
                    },
                    std::chrono::seconds(10)))
                throw std::runtime_error("Couldn't open websocket connection");
        }
        else
            throw std::runtime_error("Don't know how to open " + target);

        return result;
    }

    EchoClientWebSocket::EchoClientWebSocket(services::ConnectionFactoryWithNameResolver& connectionFactory,
        hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::BoundedString url, uint16_t port)
        : url(url)
        , port(port)
        , clientConnector(connectionFactory)
        , httpClientInitiationCreator(
              [this](infra::Optional<services::HttpClientWebSocketInitiation>& value, services::WebSocketClientObserverFactory& clientObserverFactory,
                  services::HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)
              {
                  value.Emplace(clientObserverFactory, clientConnector, result, randomDataGenerator);
              })
        , webSocketFactory(randomDataGenerator, { httpClientInitiationCreator })
    {
        webSocketFactory.Connect(*this);
    }

    void EchoClientWebSocket::OnDone(const OnDoneType& onDone)
    {
        this->onDone = onDone;
    }

    infra::BoundedString EchoClientWebSocket::Url() const
    {
        return url;
    }

    uint16_t EchoClientWebSocket::Port() const
    {
        return port;
    }

    void EchoClientWebSocket::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver)
    {
        auto echoConnectionPtr = echoConnection.Emplace(serializerFactory);
        createdClientObserver(echoConnectionPtr);
        onDone(*echoConnectionPtr);
    }

    void EchoClientWebSocket::ConnectionFailed(ConnectFailReason reason)
    {
        throw std::runtime_error("Creating ECHO over WebSocket failed");
    }

    EchoClientTcp::EchoClientTcp(services::ConnectionFactoryWithNameResolver& connectionFactory, infra::BoundedConstString hostname, uint16_t port)
        : hostname(hostname)
        , port(port)
    {
        connectionFactory.Connect(*this);
    }

    void EchoClientTcp::OnDone(const OnDoneType& onDone)
    {
        this->onDone = onDone;
    }

    infra::BoundedConstString EchoClientTcp::Hostname() const
    {
        return hostname;
    }

    uint16_t EchoClientTcp::Port() const
    {
        return port;
    }

    void EchoClientTcp::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        auto echoConnectionPtr = echoConnection.Emplace(serializerFactory);
        createdObserver(echoConnectionPtr);
        onDone(*echoConnectionPtr);
    }

    void EchoClientTcp::ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason)
    {
        throw std::runtime_error("Creating ECHO over TCP/IP failed");
    }
}
