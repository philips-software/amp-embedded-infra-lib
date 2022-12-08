#include "examples/network_example/Network.hpp"
#include "services/network/BonjourServer.hpp"

extern "C" int Main(main_::Network& network, int argc, const char* argv[], const char* env[])
{
    static infra::BoundedString::WithStorage<32> name{ "discovery" };
    static services::DnsHostnameInPartsHelper<1> text{ services::DnsHostnameInParts(name) };
    static services::IPv4Address ip{ 127, 0, 0, 1 };
    static services::BonjourServer server(network.DatagramFactory(), network.Multicast(), name, "", "", infra::MakeOptional(ip), infra::none, 80, text);

    network.Run();

    return 0;
}
