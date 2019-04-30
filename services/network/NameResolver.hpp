#ifndef SERVICES_NAME_RESOLVER_HPP
#define SERVICES_NAME_RESOLVER_HPP

#include "infra/util/IntrusiveList.hpp"
#include "services/network/Address.hpp"

namespace services
{
    class NameResolverResult
        : public infra::IntrusiveList<NameResolverResult>::NodeType
    {
    protected:
        NameResolverResult() = default;
        NameResolverResult(const NameResolverResult& other) = delete;
        NameResolverResult& operator=(const NameResolverResult& other) = delete;
        ~NameResolverResult() = default;

    public:
        virtual infra::BoundedConstString Hostname() const = 0;
        virtual IPVersions Versions() const { return IPVersions::ipv4; }
        virtual void NameLookupDone(IPAddress address) = 0;
        virtual void NameLookupFailed() = 0;
    };

    class NameResolver
    {
    protected:
        NameResolver() = default;
        NameResolver(const NameResolver& other) = delete;
        NameResolver& operator=(const NameResolver& other) = delete;
        ~NameResolver() = default;

    public:
        virtual void Lookup(NameResolverResult& result) = 0;
        virtual void CancelLookup(NameResolverResult& result) = 0;
    };
}

#endif
