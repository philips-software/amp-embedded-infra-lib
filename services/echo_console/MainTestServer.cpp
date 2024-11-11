#ifndef MAIN_ECHO_TEST_TCP_SERVER_HPP
#define MAIN_ECHO_TEST_TCP_SERVER_HPP

#include "args.hxx"
#include "echo/TracingServiceDiscovery.pb.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/util/SharedPtr.hpp"
#include "services/network/Connection.hpp"
#include <ostream>
#include <vector>
#ifdef EMIL_HAL_WINDOWS
#include "hal/windows/UartWindows.hpp"
#else
#include "hal/unix/UartUnix.hpp"
#endif
#include "infra/stream/IoOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/util/Tokenizer.hpp"
#include "protobuf/echo/Serialization.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "services/echo_console/Console.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/TracingEchoOnConnection.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/util/MessageCommunicationCobs.hpp"
#include "services/util/MessageCommunicationWindowed.hpp"
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>

class EchoServerConnectionObserver
    : private services::MethodSerializerFactory::OnHeap
    , public services::TracingEchoOnConnection
{
public:
    EchoServerConnectionObserver(services::Tracer& tracer);

private:
    service_discovery::ServiceDiscoveryTracer serviceDiscoveryTracer;
    application::ServiceDiscoveryEcho serviceDiscoveryEcho;
};

EchoServerConnectionObserver::EchoServerConnectionObserver(services::Tracer& tracer)
    : services::TracingEchoOnConnection(tracer, *this)
    , serviceDiscoveryTracer(*this)
    , serviceDiscoveryEcho(*this)
{};

class EchoServerConnection
    : public services::ServerConnectionObserverFactory
{
public:
    EchoServerConnection(services::ConnectionFactory& connectionFactory, services::Tracer& tracer);

    void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address) override;

private:
    infra::SharedPtr<void> listenPort;
    infra::SharedOptional<EchoServerConnectionObserver> observer;
    services::Tracer& tracer;
};

EchoServerConnection::EchoServerConnection(services::ConnectionFactory& connectionFactory, services::Tracer& tracer)
    : listenPort(connectionFactory.Listen(1234, *this))
    , tracer(tracer)
{}

void EchoServerConnection::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address)
{
    tracer.Trace() << "Connection accepted";
    tracer.Trace();
    createdObserver(observer.Emplace(tracer));
}

int main(int argc, char* argv[], const char* env[])
{
    infra::IoOutputStream ioOutputStream;
    services::Tracer tracer(ioOutputStream);
    services::SetGlobalTracerInstance(tracer);

    tracer.Trace() << "Starting echo server";

    main_::NetworkAdapter network;
    EchoServerConnection echoServerConnection(network.ConnectionFactory(), tracer);

    network.Run();
}

#endif // MAIN_ECHO_TEST_TCP_SERVER_HPP
