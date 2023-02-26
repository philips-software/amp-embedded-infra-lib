#include "osal/freertos/LowPowerStrategyFreeRtos.hpp"

namespace hal
{
    LowPowerStrategyFreeRtos::LowPowerStrategyFreeRtos(infra::MainClockReference& mainClock)
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
    }

    void LowPowerStrategyFreeRtos::Idle(const infra::EventDispatcherWorker& eventDispatcher)
    {
        xSemaphoreTake(semaphore, portMAX_DELAY);
    }
}
