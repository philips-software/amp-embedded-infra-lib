#ifndef TIMER_SERVICE_FREE_RTOS_HPP
#define TIMER_SERVICE_FREE_RTOS_HPP

#include "infra/timer/TickOnInterruptTimerService.hpp"
#include "infra/util/InterfaceConnector.hpp"

extern "C" uint32_t HAL_GetTick();

namespace hal
{
    class TimerServiceFreeRtos
        : public infra::InterfaceConnector<TimerServiceFreeRtos>
        , public infra::TickOnInterruptTimerService
    {
    public:
        TimerServiceFreeRtos(uint32_t id = infra::systemTimerServiceId);

        void ApplicationTickHook();

    private:
        friend uint32_t::HAL_GetTick();
    };
}

#endif
