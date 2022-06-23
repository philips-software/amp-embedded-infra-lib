#include "TimerServiceFreeRtos.hpp"

namespace hal
{
    TimerServiceFreeRtos::TimerServiceFreeRtos(uint32_t id)
        : infra::TickOnInterruptTimerService(id, std::chrono::milliseconds(1))
    {}
}

extern "C" uint32_t HAL_GetTick()
{
    if (hal::TimerServiceFreeRtos::InstanceSet())
        return std::chrono::duration_cast<std::chrono::milliseconds>(hal::TimerServiceFreeRtos::Instance().Now().time_since_epoch()).count();
    else
        return 0;
}

extern "C" void vApplicationTickHook()
{
    hal::TimerServiceFreeRtos::Instance().SystemTickInterrupt();
}
