#ifndef INFRA_PROXY_CREATOR_SHARED_HPP
#define INFRA_PROXY_CREATOR_SHARED_HPP

#include "infra/util/ProxyCreator.hpp"
#include <cstddef>

namespace infra
{
    template<class T, class U, class ConstructionArgs>
    class ProxyCreatorShared;

    template<class T, class U, class... ConstructionArgs>
    class ProxyCreatorShared<T, U, void(ConstructionArgs...)>
        : public Creator<T, U, void(ConstructionArgs...)>
    {
    public:
        using Creator<T, U, void(ConstructionArgs...)>::Creator;

        void Emplace(ConstructionArgs... args) override
        {
            if (referenceCount == 0)
                Creator<T, U, void(ConstructionArgs...)>::Emplace(args...);

            ++referenceCount;
        }

        void Destroy() override
        {
            --referenceCount;

            if (referenceCount == 0)
                Creator<T, U, void(ConstructionArgs...)>::Destroy();
        }

    private:
        std::size_t referenceCount = 0;
    };
}

#endif
