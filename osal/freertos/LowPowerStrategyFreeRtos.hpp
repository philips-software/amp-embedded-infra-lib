#ifndef OSAL_LOW_POWER_STRATEGY_FREE_RTOS_HPP
#define OSAL_LOW_POWER_STRATEGY_FREE_RTOS_HPP

#include "infra/event/LowPowerEventDispatcher.hpp"

extern "C"
{
#include "FreeRTOS.h"
#include "semphr.h"
}

namespace hal
{
    class LowPowerStrategyFreeRtos
        : public infra::LowPowerStrategy
    {
    public:
        LowPowerStrategyFreeRtos();
        virtual ~LowPowerStrategyFreeRtos();

        virtual void RequestExecution() override;
        virtual void Idle(const infra::EventDispatcherWorker& eventDispatcher) override;

    private:
        SemaphoreHandle_t semaphore = nullptr;
    };
}

#endif
