#ifndef SERVICES_BLE_STATE_GUARD_HPP
#define SERVICES_BLE_STATE_GUARD_HPP

#include "infra/util/MemoryRange.hpp"
#include "services/ble/Gap.hpp"

namespace services
{

    class StateGuard
    {
    public:
        StateGuard() = default;
        StateGuard(const StateGuard& other) = delete;
        StateGuard& operator=(const StateGuard& other) = delete;
        virtual ~StateGuard() = default;

        virtual void AssertStateIs(infra::MemoryRange<const GapState> states) const = 0;
    };
} // namespace services

#endif // SERVICES_BLE_STATE_GUARD_HPP
