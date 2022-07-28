#include "services/network/NameResolverCache.hpp"
#include "services/util/Sha256MbedTls.hpp"

namespace services
{
    NameResolverCache::NameResolverCache(infra::BoundedVector<CacheEntry>& cache, NameResolver& resolver, infra::Duration minimumTtl)
        : cache(cache)
        , resolver(resolver)
        , minimumTtl(minimumTtl)
    {}

    void NameResolverCache::Lookup(NameResolverResult& result)
    {
        auto cacheEntry = SearchCache(result);
        if (cacheEntry != infra::none)
            result.NameLookupDone(cacheEntry->address, cacheEntry->validUntil);
        else
        {
            waiting.push_back(result);
            TryResolveNext();
        }
    }

    void NameResolverCache::CancelLookup(NameResolverResult& result)
    {
        if (!activeLookup->IsResolving(result))
        {
            assert(waiting.has_element(result));
            waiting.erase(result);
        }
        else
            activeLookup->CancelLookup();
    }

    void NameResolverCache::RemoveOneCacheEntry(infra::BoundedVector<CacheEntry>& cache)
    {
        infra::TimePoint firstToExpire = cache.front().validUntil;

        for (auto& entry : cache)
            firstToExpire = std::min(firstToExpire, entry.validUntil);

        for (auto& entry : cache)
            if (entry.validUntil == firstToExpire)
            {
                cache.erase(&entry);
                break;
            }
    }

    infra::Optional<NameResolverCache::CacheEntry> NameResolverCache::SearchCache(NameResolverResult& result) const
    {
        auto nameHash = Hash(result.Hostname());

        for (auto& entry : cache)
            if (entry.nameHash == nameHash)
                return infra::MakeOptional(entry);

        return infra::none;
    }

    std::array<uint8_t, 16> NameResolverCache::Hash(infra::BoundedConstString name) const
    {
        union
        {
            struct
            {
                std::array<uint8_t, 16> first;
                std::array<uint8_t, 16> second;
            } hash_parts;
            std::array<uint8_t, 32> hash;
        } result;

        Sha256MbedTls sha256;
        result.hash = sha256.Calculate(infra::StringAsByteRange(name));

        return result.hash_parts.first;
    }

    void NameResolverCache::TryResolveNext()
    {
        if (activeLookup == infra::none && !waiting.empty())
        {
            activeLookup.Emplace(*this, waiting.front());
            waiting.pop_front();
        }
    }

    void NameResolverCache::NameLookupSuccess(NameResolverResult& result, IPAddress address, infra::TimePoint validUntil)
    {
        validUntil = std::max(infra::Now() + minimumTtl, validUntil);
        TryAddToCache(result.Hostname(), address, validUntil);
        NameLookupDone([&result, &address, &validUntil]() { result.NameLookupDone(address, validUntil); });
    }

    void NameResolverCache::NameLookupFailed(NameResolverResult& result)
    {
        NameLookupDone([&result]() { result.NameLookupFailed(); });
    }

    void NameResolverCache::NameLookupCancelled()
    {
        NameLookupDone([]() {});
    }

    void NameResolverCache::NameLookupDone(const infra::Function<void(), 3 * sizeof(void*)>& observerCallback)
    {
        activeLookup = infra::none;
        observerCallback();
        TryResolveNext();
    }

    void NameResolverCache::TryAddToCache(infra::BoundedConstString name, IPAddress address, infra::TimePoint validUntil)
    {
        if (cache.full())
            RemoveOneCacheEntry(cache);

        AddToCache(name, address, validUntil);
    }

    void NameResolverCache::AddToCache(infra::BoundedConstString name, IPAddress address, infra::TimePoint validUntil)
    {
        cache.emplace_back(CacheEntry{ address, validUntil, Hash(name) });
        Cleanup();
    }

    void NameResolverCache::Cleanup()
    {
        infra::TimePoint now = infra::Now();
        infra::TimePoint cleanupTime = infra::TimePoint::max();

        for (auto entry = cache.begin(); entry != cache.end(); )
        {
            if (entry->validUntil <= now)
                entry = cache.erase(entry);
            else
            {
                cleanupTime = std::min(cleanupTime, entry->validUntil);
                ++entry;
            }
        }

        cleanupTimer.Start(cleanupTime, [this]() { Cleanup(); });
    }

    NameResolverCache::ActiveLookup::ActiveLookup(NameResolverCache& nameResolverCache, NameResolverResult& resolving)
        : nameResolverCache(nameResolverCache)
        , resolving(resolving)
    {
        nameResolverCache.resolver.Lookup(*this);
    }

    bool NameResolverCache::ActiveLookup::IsResolving(NameResolverResult& resolving) const
    {
        return &resolving == &this->resolving;
    }

    void NameResolverCache::ActiveLookup::CancelLookup()
    {
        nameResolverCache.resolver.CancelLookup(*this);
        nameResolverCache.NameLookupCancelled();
    }

    infra::BoundedConstString NameResolverCache::ActiveLookup::Hostname() const
    {
        return resolving.Hostname();
    }

    IPVersions NameResolverCache::ActiveLookup::Versions() const
    {
        return resolving.Versions();
    }

    void NameResolverCache::ActiveLookup::NameLookupDone(IPAddress address, infra::TimePoint validUntil)
    {
        nameResolverCache.NameLookupSuccess(resolving, address, validUntil);
    }

    void NameResolverCache::ActiveLookup::NameLookupFailed()
    {
        nameResolverCache.NameLookupFailed(resolving);
    }
}
