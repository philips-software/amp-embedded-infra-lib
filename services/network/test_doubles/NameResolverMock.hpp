#ifndef NETWORK_NAME_RESOLVER_MOCK_MOCK_HPP
#define NETWORK_NAME_RESOLVER_MOCK_MOCK_HPP

#include "services/network/NameResolver.hpp"
#include "gmock/gmock.h"

namespace services
{
    class NameResolverResultMock
        : public NameResolverResult
    {
    public:
        MOCK_CONST_METHOD0(Hostname, infra::BoundedConstString());
        MOCK_CONST_METHOD0(Versions, IPVersions());
        MOCK_METHOD2(NameLookupDone, void(IPAddress address, infra::TimePoint validUntil));
        MOCK_METHOD0(NameLookupFailed, void());
    };

    class NameResolverMock
        : public NameResolver
    {
    public:
        MOCK_METHOD1(Lookup, void(NameResolverResult& result));
        MOCK_METHOD1(CancelLookup, void(NameResolverResult& result));
    };
}

#endif
