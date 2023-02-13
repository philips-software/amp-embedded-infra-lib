#include "osal/Osal.hpp"
#include <cassert>

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"

    void vAssertCalled(const char* const filename, unsigned long line)
    {}

    void vApplicationMallocFailedHook()
    {}
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
