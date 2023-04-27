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
