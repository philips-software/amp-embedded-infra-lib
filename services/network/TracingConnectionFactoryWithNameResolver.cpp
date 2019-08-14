#include "services/network/TracingConnectionFactoryWithNameResolver.hpp"

namespace services
{
    TracingConnectionFactoryWithNameResolver::TracingConnectionFactoryWithNameResolver(infra::BoundedList<Action>& actions, ConnectionFactory& connectionFactory, NameResolver& nameLookup, services::Tracer& tracer)
        : ConnectionFactoryWithNameResolverImpl(actions, connectionFactory, nameLookup)
        , tracer(tracer)
    {}

    void TracingConnectionFactoryWithNameResolver::NameLookupFailed()
    {
        tracer.Trace() << "Name lookup failed";
    }

    void TracingConnectionFactoryWithNameResolver::NameLookupSuccessful(IPAddress address)
    {
        tracer.Trace() << "Name lookup done: " << address;
    }
}
