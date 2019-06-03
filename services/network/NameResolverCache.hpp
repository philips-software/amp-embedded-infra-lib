#ifndef SERVICES_NAME_RESOLVER_CACHE_HPP
#define SERVICES_NAME_RESOLVER_CACHE_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/NameResolver.hpp"

namespace services
{
    class NameResolverCache
        : public NameResolver
    {
    public:
        struct CacheEntry
        {
            IPAddress address;
            infra::TimePoint validUntil;
            std::array<uint8_t, 16> nameHash;
        };

        template<std::size_t Size>
            using WithCacheSize = infra::WithStorage<NameResolverCache, infra::BoundedVector<CacheEntry>::WithMaxSize<Size>>;

        NameResolverCache(infra::BoundedVector<CacheEntry>& cache, NameResolver& resolver);

        // Implementation of NameResolver
        virtual void Lookup(NameResolverResult& result) override;
        virtual void CancelLookup(NameResolverResult& result) override;

    protected:
        virtual void RemoveOneCacheEntry(infra::BoundedVector<CacheEntry>& cache);

    private:
        class ActiveLookup
            : private NameResolverResult
        {
        public:
            ActiveLookup(NameResolverCache& nameResolverCache, NameResolverResult& resolving);

            bool IsResolving(NameResolverResult& resolving) const;
            void CancelLookup();

        private:
            virtual infra::BoundedConstString Hostname() const override;
            virtual IPVersions Versions() const override;
            virtual void NameLookupDone(IPAddress address, infra::TimePoint validUntil) override;
            virtual void NameLookupFailed() override;

        private:
            NameResolverCache& nameResolverCache;
            NameResolverResult& resolving;
        };

    private:
        infra::Optional<CacheEntry> SearchCache(NameResolverResult& result) const;
        std::array<uint8_t, 16> Hash(infra::BoundedConstString name) const;
        void TryResolveNext();
        void NameLookupSuccess(NameResolverResult& result, IPAddress address, infra::TimePoint validUntil);
        void NameLookupFailed(NameResolverResult& result);
        void NameLookupCancelled();
        void NameLookupDone(const infra::Function<void(), 3 * sizeof(void*)>& observerCallback);
        void TryAddToCache(infra::BoundedConstString name, IPAddress address, infra::TimePoint validUntil);
        void AddToCache(infra::BoundedConstString name, IPAddress address, infra::TimePoint validUntil);
        void Cleanup();

    private:
        infra::BoundedVector<CacheEntry>& cache;
        NameResolver& resolver;

        infra::IntrusiveList<NameResolverResult> waiting;
        infra::Optional<ActiveLookup> activeLookup;

        infra::TimerSingleShot cleanupTimer;
    };
}

#endif
