#ifndef INFRA_EVENT_DISPATCHER_FIXTURE_HPP
#define INFRA_EVENT_DISPATCHER_FIXTURE_HPP

#include "infra/event/EventDispatcher.hpp"

namespace infra
{
    class EventDispatcherFixture
        : public EventDispatcher::WithSize<50>
    {};
}

#endif
