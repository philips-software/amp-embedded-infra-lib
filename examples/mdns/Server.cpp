#include "services/network/BonjourServer.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"

int main(int argc, const char* argv[], const char* env[])
{
    static main_::NetworkAdapter network;
    static infra::BoundedString::WithStorage<32> name{ "discovery" };
    static services::DnsHostnameInPartsHelper<1> text{ services::DnsHostnameInParts(name) };
    static services::IPv4Address ip{ 127, 0, 0, 1 };
    static services::BonjourServer server(network.DatagramFactory(), network.Multicast(), name, "", "", infra::MakeOptional(ip), infra::none, 80, text);

    network.Run();

    return 0;
}
