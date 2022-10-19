#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/NameResolverCache.hpp"
#include "services/network/test_doubles/NameResolverMock.hpp"
#include "gmock/gmock.h"

class NameResolverCacheTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    NameResolverCacheTest()
    {
        EXPECT_CALL(result1, Hostname()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(hostname1));
        EXPECT_CALL(result1, Versions()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(services::IPVersions::both));
        EXPECT_CALL(result2, Hostname()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(hostname2));
        EXPECT_CALL(result2, Versions()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(services::IPVersions::both));
    }

    void Lookup(services::NameResolverResultMock& result)
    {
        EXPECT_CALL(resolver, Lookup(testing::_)).WillOnce(infra::SaveRef<0>(&lookupResult));
        resolverCache.Lookup(result);
    }

    void CancelLookup(services::NameResolverResultMock& result)
    {
        EXPECT_CALL(resolver, CancelLookup(testing::_));
        resolverCache.CancelLookup(result);
    }

    void ExpectNameLookupDone(services::NameResolverResultMock& result, services::IPAddress address, infra::Duration ttl = std::chrono::hours(1))
    {
        EXPECT_CALL(result, NameLookupDone(address, infra::Now() + ttl));
    }

    void NameLookupDone(services::NameResolverResultMock& result, services::IPAddress address, infra::Duration ttl = std::chrono::hours(1))
    {
        ExpectNameLookupDone(result, address, ttl);
        lookupResult->NameLookupDone(address, infra::Now() + ttl);
    }

    void NameLookupFailed(services::NameResolverResultMock& result)
    {
        EXPECT_CALL(result, NameLookupFailed());
        lookupResult->NameLookupFailed();
    }

    void AddToCache(infra::BoundedConstString hostname, services::IPAddress address, infra::Duration ttl = std::chrono::hours(1))
    {
        testing::StrictMock<services::NameResolverResultMock> result;

        EXPECT_CALL(result, Hostname()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(hostname));
        EXPECT_CALL(result, Versions()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(services::IPVersions::both));

        Lookup(result);
        NameLookupDone(result, address, ttl);
    }

    infra::BoundedConstString hostname1 = "hostname1";
    infra::BoundedConstString hostname2 = "hostname2";
    infra::BoundedConstString hostname3 = "hostname3";
    const services::IPAddress address1{ services::IPv4Address{ 1, 2, 3, 4 } };

    testing::StrictMock<services::NameResolverMock> resolver;
    services::NameResolverCache::WithCacheSize<2> resolverCache{ resolver };
    testing::StrictMock<services::NameResolverResultMock> result1;
    testing::StrictMock<services::NameResolverResultMock> result2;
    services::NameResolverResult* lookupResult = nullptr;
};

TEST_F(NameResolverCacheTest, start_lookup)
{
    Lookup(result1);
}

TEST_F(NameResolverCacheTest, lookup_is_successful)
{
    Lookup(result1);
    NameLookupDone(result1, address1);
}

TEST_F(NameResolverCacheTest, lookup_failed)
{
    Lookup(result1);
    NameLookupFailed(result1);
}

TEST_F(NameResolverCacheTest, second_lookup_is_queued)
{
    Lookup(result1);
    resolverCache.Lookup(result2);
}

TEST_F(NameResolverCacheTest, second_lookup_is_done_after_first)
{
    Lookup(result1);
    resolverCache.Lookup(result2);

    EXPECT_CALL(resolver, Lookup(testing::_));
    NameLookupDone(result1, address1);
}

TEST_F(NameResolverCacheTest, cancel_lookup)
{
    Lookup(result1);
    CancelLookup(result1);
}

TEST_F(NameResolverCacheTest, cancel_queued_lookup)
{
    Lookup(result1);
    resolverCache.Lookup(result2);

    resolverCache.CancelLookup(result2);

    NameLookupDone(result1, address1);
}

TEST_F(NameResolverCacheTest, cancel_lookup_starts_next_lookup)
{
    Lookup(result1);
    resolverCache.Lookup(result2);

    EXPECT_CALL(resolver, Lookup(testing::_));
    CancelLookup(result1);
}

TEST_F(NameResolverCacheTest, lookup_from_cache)
{
    AddToCache(hostname1, address1);

    ExpectNameLookupDone(result1, address1);
    resolverCache.Lookup(result1);
}

TEST_F(NameResolverCacheTest, lookup_from_cache_during_other_lookup)
{
    AddToCache(hostname1, address1);

    Lookup(result2);

    ExpectNameLookupDone(result1, address1);
    resolverCache.Lookup(result1);
}

TEST_F(NameResolverCacheTest, fill_cache)
{
    AddToCache(hostname1, address1);
    AddToCache(hostname2, address1);
    AddToCache(hostname3, address1);
}

TEST_F(NameResolverCacheTest, oldest_is_removed_on_overflow)
{
    AddToCache(hostname1, address1);
    ForwardTime(std::chrono::seconds(1));
    AddToCache(hostname2, address1);
    AddToCache(hostname3, address1);

    Lookup(result1);
}

TEST_F(NameResolverCacheTest, no_lookup_from_cache_if_expired)
{
    AddToCache(hostname1, address1);

    ForwardTime(std::chrono::hours(2));

    Lookup(result1);
}

TEST_F(NameResolverCacheTest, minimum_ttl_is_applied)
{
    Lookup(result1);
    ExpectNameLookupDone(result1, address1, std::chrono::minutes(10));
    lookupResult->NameLookupDone(address1, infra::Now() + std::chrono::minutes(5));
}
