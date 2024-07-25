#ifndef SERVICES_TRACING_ECHO_ON_SESAME_HPP
#define SERVICES_TRACING_ECHO_ON_SESAME_HPP

#include "protobuf/echo/TracingEcho.hpp"
#include "services/util/EchoOnSesame.hpp"

namespace services
{
    using TracingEchoOnSesame = TracingEchoOnStreamsDescendant<EchoOnSesame>;
}

#endif
