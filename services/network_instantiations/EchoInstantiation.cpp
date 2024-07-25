#include "services/network_instantiations/EchoInstantiation.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/timer/Waiting.hpp"
#include "services/tracer/TracingEchoInstantiation.hpp"

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

    std::pair<std::shared_ptr<services::Echo>, std::shared_ptr<services::TracingEchoOnStreams>> OpenTracingEcho(infra::BoundedString target, services::ConnectionFactoryWithNameResolver& connectionFactory, services::Tracer& tracer)
    {
        std::shared_ptr<services::Echo> resultEcho;
        std::shared_ptr<services::TracingEchoOnStreams> resultTracer;

        if (target.substr(0, 3) == "COM" || target.substr(0, 4) == "/dev")
        {
            auto echoClient = std::make_shared<main_::TracingEchoOnUart<256>>(target, tracer);
            resultEcho = std::shared_ptr<services::Echo>(echoClient, &echoClient->echo);
            resultTracer = std::shared_ptr<services::TracingEchoOnStreams>(echoClient, &echoClient->echoOnSesame.echo);
        }
        else if (services::SchemeFromUrl(target) == "ws")
        {
            if (!infra::host::WaitUntilDone(
                    [&](const std::function<void()>& done)
                    {
                        static hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

                        auto echoClient = std::make_shared<TracingEchoClientWebSocket>(connectionFactory, randomDataGenerator, services::HostFromUrl(target), services::PortFromUrl(target).ValueOr(80), tracer);
                        echoClient->OnDone([&, echoClient](services::Echo& echo, services::TracingEchoOnStreams& echoTracer)
                            {
                                resultEcho = std::shared_ptr<services::Echo>(echoClient, &echo);
                                resultTracer = std::shared_ptr<services::TracingEchoOnStreams>(echoClient, &echoTracer);
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
                        auto echoClient = std::make_shared<TracingEchoClientTcp>(connectionFactory, services::HostFromUrl(target), services::PortFromUrl(target).ValueOr(1234), tracer);
                        echoClient->OnDone([&, echoClient](services::Echo& echo, services::TracingEchoOnStreams& echoTracer)
                            {
                                resultEcho = std::shared_ptr<services::Echo>(echoClient, &echo);
                                resultTracer = std::shared_ptr<services::TracingEchoOnStreams>(echoClient, &echoTracer);
                                done();
                            });
                    },
                    std::chrono::seconds(10)))
                throw std::runtime_error("Couldn't open websocket connection");
        }
        else
            throw std::runtime_error("Don't know how to open " + target);

        return std::make_pair(resultEcho, resultTracer);
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

    TracingEchoClientWebSocket::TracingEchoClientWebSocket(services::ConnectionFactoryWithNameResolver& connectionFactory,
        hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::BoundedString url, uint16_t port, services::Tracer& tracer)
        : EchoClientWebSocket(connectionFactory, randomDataGenerator, url, port)
        , tracer(tracer)
    {}

    void TracingEchoClientWebSocket::OnDone(const OnDoneType& onDone)
    {
        this->onDone = onDone;
    }

    void TracingEchoClientWebSocket::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> client)>&& createdClientObserver)
    {
        auto echoConnectionPtr = echoConnection.Emplace(serializerFactory, services::echoErrorPolicyAbortOnMessageFormatError, tracer);
        createdClientObserver(echoConnectionPtr);
        onDone(*echoConnectionPtr, *echoConnectionPtr);
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

    TracingEchoClientTcp::TracingEchoClientTcp(services::ConnectionFactoryWithNameResolver& connectionFactory, infra::BoundedConstString hostname, uint16_t port, services::Tracer& tracer)
        : EchoClientTcp(connectionFactory, hostname, port)
        , tracer(tracer)
    {}

    void TracingEchoClientTcp::OnDone(const OnDoneType& onDone)
    {
        this->onDone = onDone;
    }

    void TracingEchoClientTcp::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        auto echoConnectionPtr = echoConnection.Emplace(serializerFactory, services::echoErrorPolicyAbortOnMessageFormatError, tracer);
        createdObserver(echoConnectionPtr);
        onDone(*echoConnectionPtr, *echoConnectionPtr);
    }
}
