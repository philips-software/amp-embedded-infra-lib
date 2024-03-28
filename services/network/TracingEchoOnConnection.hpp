#ifndef SERVICES_TRACING_ECHO_ON_CONNECTION_HPP
#define SERVICES_TRACING_ECHO_ON_CONNECTION_HPP

#include "protobuf/echo/TracingEcho.hpp"
#include "services/network/EchoOnConnection.hpp"

namespace services
{
    using TracingEchoOnConnection = TracingEchoOnStreamsDescendant<EchoOnConnection>;
}

#endif
