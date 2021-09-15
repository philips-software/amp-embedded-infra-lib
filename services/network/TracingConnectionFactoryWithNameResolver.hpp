#ifndef SERVICES_TRACING_CONNECTION_FACTORY_WITH_NAME_RESOLVER_HPP
#define SERVICES_TRACING_CONNECTION_FACTORY_WITH_NAME_RESOLVER_HPP

#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingConnectionFactoryWithNameResolver
        : public ConnectionFactoryWithNameResolverImpl
    {
    public:
        template<std::size_t NumParallelActions>
        using WithStorage = infra::WithStorage<TracingConnectionFactoryWithNameResolver, infra::BoundedList<Action>::WithMaxSize<NumParallelActions>>;

        TracingConnectionFactoryWithNameResolver(infra::BoundedList<Action>& actions, ConnectionFactory& connectionFactory, NameResolver& nameLookup, services::Tracer& tracer);

    protected:
        virtual void NameLookupFailed() override;
        virtual void NameLookupSuccessful(IPAddress address) override;

    private:
        services::Tracer& tracer;
    };
}

#endif
