#ifndef INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_FIXTURE_HPP
#define INFRA_EVENT_DISPATCHER_WITH_WEAK_PTR_FIXTURE_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"

namespace infra
{
    class EventDispatcherWithWeakPtrFixture
        : public EventDispatcherWithWeakPtr::WithSize<50>
    {};
}

#endif
