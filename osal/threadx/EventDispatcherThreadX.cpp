#include "osal/threadx/EventDispatcherThreadX.hpp"

namespace main_
{
    constexpr static ULONG Execute{ 0x01 };

    void EventDispatcherThreadXWorker::RequestExecution()
    {
        if (tx_event_flags_set(&flags, Execute, TX_OR) != TX_SUCCESS)
            std::abort();
    }

    void EventDispatcherThreadXWorker::Idle()
    {
        ULONG events;
        if (tx_event_flags_get(&flags, Execute, TX_OR_CLEAR, &events, TX_WAIT_FOREVER) != TX_SUCCESS)
            std::abort();
    }

    TX_EVENT_FLAGS_GROUP EventDispatcherThreadXWorker::init()
    {
        TX_EVENT_FLAGS_GROUP flags{};
        if (tx_event_flags_create(&flags, const_cast<char*>("")) != TX_SUCCESS)
            std::abort();

        return flags;
    }
}
