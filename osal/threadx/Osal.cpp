#include "osal/Osal.hpp"

extern "C"
{
#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"

    VOID tx_application_define(VOID* first_unused_memory)
    {}
}

namespace osal
{
    void Init()
    {
        tx_kernel_enter();
        _tx_thread_system_state = TX_INITIALIZE_IN_PROGRESS;
    }

    void Run()
    {
        _tx_thread_system_state = TX_INITIALIZE_IS_FINISHED;

        TX_PORT_SPECIFIC_PRE_SCHEDULER_INITIALIZATION

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
        _tx_execution_initialize();
#endif

        _tx_thread_schedule();

#ifdef TX_SAFETY_CRITICAL
        TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);
#endif
    }
}
