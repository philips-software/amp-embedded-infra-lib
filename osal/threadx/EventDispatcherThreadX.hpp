#ifndef OSAL_EVENT_DISPATCHER_THREADX_HPP
#define OSAL_EVENT_DISPATCHER_THREADX_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "tx_api.h"

namespace hal
{
    class EventDispatcherThreadXWorker
        : public infra::EventDispatcherWithWeakPtrWorker
    {
    public:
        using EventDispatcherWithWeakPtrWorker::EventDispatcherWithWeakPtrWorker;

        void RequestExecution() override;
        void Idle() override;

    private:
        TX_EVENT_FLAGS_GROUP flags{ init() };

        static TX_EVENT_FLAGS_GROUP init();
    };

    using EventDispatcherThreadX = infra::EventDispatcherWithWeakPtrConnector<EventDispatcherThreadXWorker>;
}

#endif
