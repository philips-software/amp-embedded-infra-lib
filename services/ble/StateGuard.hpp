#ifndef SERVICES_BLE_STATE_GUARD_HPP
#define SERVICES_BLE_STATE_GUARD_HPP

#include "infra/stream/StringOutputStream.hpp"
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

        bool StateIs(std::initializer_list<GapState> states) const
        {
            auto currentState = DetermineCurrentState();
            return std::any_of(states.begin(), states.end(), [currentState](auto state)
                {
                    return state == currentState;
                });
        }

        void AssertStateIs(std::initializer_list<GapState> states) const
        {
            if (!StateIs(states))
            {
                infra::StringOutputStream::WithStorage<16> stream;
                for (auto it = states.begin(); it != states.end(); ++it)
                {
                    if (it != states.begin())
                        stream << ",";
                    stream << static_cast<int>(*it);
                }

                really_assert_with_msg(false,
                    "Unexpected state found: %d, expected: [%.*s]",
                    static_cast<int>(DetermineCurrentState()),
                    static_cast<int>(stream.Storage().size()),
                    stream.Storage().data());
            }
        }

    protected:
        virtual GapState DetermineCurrentState() const = 0;
    };
} // namespace services

#endif // SERVICES_BLE_STATE_GUARD_HPP
