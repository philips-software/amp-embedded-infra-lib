#ifndef INFRA_WAITING_HPP
#define INFRA_WAITING_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/Function.hpp"
#include <functional>
#include <utility>

namespace infra
{
    constexpr inline infra::Duration defaultTimeout{ std::chrono::seconds{ 1 } };

    bool WaitUntilDone(const infra::Function<void(const infra::Function<void()>&)>& action, infra::Duration timeout = defaultTimeout);
    bool WaitFor(const infra::Function<bool()>& pred, infra::Duration timeout = defaultTimeout);

    // On host platforms, std::function allows more freedom than infra::Function
    namespace host
    {
        bool WaitUntilDone(const std::function<void(const std::function<void()>&)>& action, infra::Duration timeout = defaultTimeout);
        bool WaitFor(const std::function<bool()>& pred, infra::Duration timeout = defaultTimeout);
    }
}

#endif
