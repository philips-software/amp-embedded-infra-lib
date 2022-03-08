#ifndef NETWORK_CERTIFICATES_HPP
#define NETWORK_CERTIFICATES_HPP

#include "infra/util/BoundedString.hpp"

namespace services
{
    extern infra::BoundedConstString testCaCertificate;
    extern infra::BoundedConstString testServerCertificate;
    extern infra::BoundedConstString testServerKey;
    extern infra::BoundedConstString testClientCertificate;
    extern infra::BoundedConstString testClientKey;
}

#endif
