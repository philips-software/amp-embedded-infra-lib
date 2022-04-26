#ifndef INFRA_TIMER_SERVICE_MANAGER_HPP
#define INFRA_TIMER_SERVICE_MANAGER_HPP

#include "infra/timer/TimerService.hpp"
#include "infra/util/InterfaceConnector.hpp"

namespace infra
{
    class TimerServiceManager
        : public infra::InterfaceConnector<TimerServiceManager>
    {
    public:
        void RegisterTimerService(TimerService& timerService);
        void UnregisterTimerService(TimerService& timerService);

        TimerService& GetTimerService(uint32_t id);

    private:
        infra::IntrusiveForwardList<TimerService> timerServices;
    };
}

#endif
