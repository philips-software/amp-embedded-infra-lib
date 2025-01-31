#ifndef MAIN_ECHO_TEST_TCP_SERVER_HPP
#define MAIN_ECHO_TEST_TCP_SERVER_HPP

#include "echo/TracingServiceDiscovery.pb.hpp"
#include "infra/stream/IoOutputStream.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Serialization.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "services/echo_console/Console.hpp"
#include "services/network/Connection.hpp"
#include "services/network/TracingEchoOnConnection.hpp"
#include "services/tracer/GlobalTracer.hpp"

class EchoServerConnectionObserver
    : private services::MethodSerializerFactory::OnHeap
    , public services::TracingEchoOnConnection
{
public:
    EchoServerConnectionObserver(services::Tracer& tracer);

private:
    service_discovery::ServiceDiscoveryTracer serviceDiscoveryTracer;
    service_discovery::ServiceDiscoveryResponseTracer serviceDiscoveryResponseTracer;
    application::ServiceDiscoveryEcho serviceDiscoveryEcho;
};

EchoServerConnectionObserver::EchoServerConnectionObserver(services::Tracer& tracer)
    : services::TracingEchoOnConnection(tracer, *this)
    , serviceDiscoveryTracer(*this)
    , serviceDiscoveryResponseTracer(*this)
    , serviceDiscoveryEcho(*this){};

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
