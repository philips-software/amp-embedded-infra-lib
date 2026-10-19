#ifndef INFRA_CREATOR_SHARED_HPP
#define INFRA_CREATOR_SHARED_HPP

#include "infra/util/ProxyCreator.hpp"
#include <cstddef>

namespace infra
{
    template<class T, class U, class ConstructionArgs>
    class CreatorShared;

    template<class T, class U, class... ConstructionArgs>
    class CreatorShared<T, U, void(ConstructionArgs...)>
        : public Creator<T, U, void(ConstructionArgs...)>
    {
    public:
        using Creator<T, U, void(ConstructionArgs...)>::Creator;

        void Emplace(ConstructionArgs... args) override
        {
            if (users == 0)
                Creator<T, U, void(ConstructionArgs...)>::Emplace(args...);

            ++users;
        }

        void Destroy() override
        {
            --users;

            if (users == 0)
                Creator<T, U, void(ConstructionArgs...)>::Destroy();
        }

    private:
        std::size_t users = 0;
    };
}

#endif
