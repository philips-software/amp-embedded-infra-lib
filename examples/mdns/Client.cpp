#include "hal/generic/TimerServiceGeneric.hpp"
#include "infra/timer/Timer.hpp"
#include "services/network/MdnsClient.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"

int main(int argc, const char* argv[], const char* env[])
{
    static hal::TimerServiceGeneric timerService;
    static main_::TracerOnIoOutputInfrastructure tracer;
    static main_::NetworkAdapter network;
    static services::MdnsClient mdns(network.DatagramFactory(), network.Multicast(), services::IPVersions::ipv4);
    static services::MdnsQueryImpl query(mdns, services::DnsType::dnsTypeA, "discovery", [](infra::ConstByteRange data)
    {
        services::IPv4Address address;
        infra::Copy(data, infra::MakeByteRange(address));

        tracer.tracer.Trace() << "Found mDNS server on: " << address; 
    });

    infra::TimerRepeating queryTimer(std::chrono::seconds(5), [] { query.Ask(); });

    network.Run();

    return 0;
}
