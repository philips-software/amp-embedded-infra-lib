#ifndef INFRA_LIFETIME_HELPER_HPP
#define INFRA_LIFETIME_HELPER_HPP

#include <memory>
#include <vector>

namespace infra
{
    class LifetimeHelper
    {
    public:
        template<class T>
        const T& KeepAlive(const T& value)
        {
            auto v = std::make_shared<T>(value);
            alive.push_back(v);
            return *v;
        }

        infra::ConstByteRange KeepBytesAlive(const std::vector<uint8_t>& value)
        {
            auto v = std::make_shared<std::vector<uint8_t>>(value);
            alive.push_back(v);
            return *v;
        }

    private:
        std::vector<std::shared_ptr<void>> alive;
    };
}

#endif
