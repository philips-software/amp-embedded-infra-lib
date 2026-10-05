#include "osal/freertos/LowPowerStrategyFreeRtos.hpp"
extern "C"
{
#include "portmacro.h"
}

namespace hal
{
    LowPowerStrategyFreeRtos::LowPowerStrategyFreeRtos()
        : semaphore(xSemaphoreCreateBinary())
    {}

    LowPowerStrategyFreeRtos::~LowPowerStrategyFreeRtos()
    {
        vSemaphoreDelete(semaphore);
    }

    void LowPowerStrategyFreeRtos::RequestExecution()
    {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(semaphore, &higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

    void LowPowerStrategyFreeRtos::Idle(const infra::EventDispatcherWorker& eventDispatcher)
    {
        xSemaphoreTake(semaphore, portMAX_DELAY);
    }
}
