#include "examples/network_example/Network.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"
#include "services/network_win/NameLookupWin.hpp"

class NetworkAdapter
    : public main_::Network
{
public:
    services::ConnectionFactory& ConnectionFactory() override;
    services::DatagramFactory& DatagramFactory() override;
    services::Multicast& Multicast() override;
    services::NameResolver& NameResolver() override;

    void Run() override;

private:
    services::EventDispatcherWithNetwork network;
    services::NameLookupWin nameResolver;
};

services::ConnectionFactory& NetworkAdapter::ConnectionFactory()
{
    return network;
}

services::DatagramFactory& NetworkAdapter::DatagramFactory()
{
    return network;
}

services::Multicast& NetworkAdapter::Multicast()
{
    return network;
}

services::NameResolver& NetworkAdapter::NameResolver()
{
    return nameResolver;
}

void NetworkAdapter::Run()
{
    network.Run();
}

int main(int argc, const char* argv[], const char* env[])
{
    static NetworkAdapter network;
    return Main(network, argc, argv, env);
}
