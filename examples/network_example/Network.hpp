#ifndef EXAMPLES_NETWORK_EXAMPLE_NETWORK_HPP
#define EXAMPLES_NETWORK_EXAMPLE_NETWORK_HPP

#include "services/network/Connection.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Multicast.hpp"
#include "services/network/NameResolver.hpp"

namespace main_
{
    class Network
    {
    public:
        virtual services::ConnectionFactory& ConnectionFactory() = 0;
        virtual services::DatagramFactory& DatagramFactory() = 0;
        virtual services::Multicast& Multicast() = 0;
        virtual services::NameResolver& NameResolver() = 0;

        virtual void Run() = 0;
    };
}

extern "C" int Main(main_::Network& network, int argc, const char* argv[], const char* env[]);

#endif
