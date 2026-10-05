#include "osal/freertos/LowPowerStrategyFreeRtos.hpp"
#include "portmacro.h"

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
        BaseType_t higherPriorityTaskWoken;
        xSemaphoreGiveFromISR(semaphore, &higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

    void LowPowerStrategyFreeRtos::Idle(const infra::EventDispatcherWorker& eventDispatcher)
    {
        xSemaphoreTake(semaphore, portMAX_DELAY);
    }
}
