#include "osal/Osal.hpp"
#include <cassert>
#include <cstdlib>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"

    void vAssertCalled(const char* const filename, unsigned long line)
    {
        std::abort();
    }

    void vApplicationMallocFailedHook()
    {
        std::abort();
    }

    void xPortPendSVHandler();
    void xPortSysTickHandler();
    void vPortSVCHandler();

    [[gnu::naked]] void SVC_Handler() { vPortSVCHandler(); }
    [[gnu::naked]] void PendSV_Handler() { xPortPendSVHandler(); }
    [[gnu::naked]] void SysTick_Handler() { xPortSysTickHandler(); };
}

namespace osal
{
    void Init()
    {}

    void Run()
    {
        vTaskStartScheduler();
    }
}
