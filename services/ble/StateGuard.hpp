#ifndef SERVICES_BLE_STATE_GUARD_HPP
#define SERVICES_BLE_STATE_GUARD_HPP

#include "infra/util/ReallyAssert.hpp"
#include "services/ble/Gap.hpp"
#include <algorithm>
#include <initializer_list>

namespace services
{

    class StateGuard
    {
    public:
        StateGuard() = default;
        StateGuard(const StateGuard& other) = delete;
        StateGuard& operator=(const StateGuard& other) = delete;
        virtual ~StateGuard() = default;

        void AssertStateIs(std::initializer_list<GapState> states) const
        {
            auto currentState = DetermineCurrentState();

            really_assert(std::any_of(states.begin(), states.end(), [currentState](auto state)
                {
                    return state == currentState;
                }));
        }

    protected:
        virtual GapState DetermineCurrentState() const = 0;
    };
} // namespace services

#endif // SERVICES_BLE_STATE_GUARD_HPP
