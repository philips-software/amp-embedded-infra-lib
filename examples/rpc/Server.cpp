#include "generated/echo/Console.pb.hpp"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/network/EchoOnConnection.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include <iostream>

class Console
    : public examples::Console
{
public:
    Console(services::Echo& echo, services::MethodDeserializerFactory& deserializerFactory)
        : examples::Console(echo, deserializerFactory)
    {}

    void Write(infra::BoundedConstString message) override
    {
        std::cout << infra::AsStdString(message) << std::endl;
        MethodDone();
    }
};

class EchoConnection
    : public services::EchoOnConnection
{
public:
    EchoConnection(services::MethodDeserializerFactory& deserializerFactory)
        : console(*this, deserializerFactory)
    {}

private:
    Console console;
};

class RpcServer
    : public services::ServerConnectionObserverFactory
{
public:
    RpcServer(services::ConnectionFactory& factory)
        : listener(factory.Listen(1234, *this))
    {}

    void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, services::IPAddress address) override
    {
        if (connection)
            connection->::services::ConnectionObserver::Subject().AbortAndDestroy();

        if (connection.Allocatable())
            createdObserver(connection.Emplace(deserializerFactory));
    }

private:
    infra::SharedPtr<void> listener;
    infra::SharedOptional<EchoConnection> connection;
    services::MethodDeserializerFactory::OnHeap deserializerFactory;
};

int main(int argc, const char* argv[], const char* env[])
{
    static hal::TimerServiceGeneric timerService;
    static main_::NetworkAdapter network;
    static RpcServer rpc(network.ConnectionFactory());

    network.Run();

    return 0;
}
