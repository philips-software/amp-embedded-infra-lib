#include "infra/timer/TimerServiceManager.hpp"
#include <cassert>

namespace infra
{
    namespace
    {
        TimerServiceManager timerServiceManager;
    }

    void TimerServiceManager::RegisterTimerService(TimerService& timerService)
    {
        timerServices.push_front(timerService);
    }

    void TimerServiceManager::UnregisterTimerService(TimerService& timerService)
    {
        timerServices.erase_slow(timerService);
    }

    TimerService& TimerServiceManager::GetTimerService(uint32_t id)
    {
        for (TimerService& timerService : timerServices)
            if (timerService.Id() == id)
                return timerService;

        abort(); // No timer service with the given id found
    }
}
