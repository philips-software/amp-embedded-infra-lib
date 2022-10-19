#ifndef SERVICES_STOPPABLE_MOCK_HPP
#define SERVICES_STOPPABLE_MOCK_HPP

#include "services/util/Stoppable.hpp"
#include "gmock/gmock.h"

namespace services
{
    class StoppableMock
        : public Stoppable
    {
    public:
        MOCK_METHOD1(Stop, void(const infra::Function<void()>& onDone));
    };
}

#endif
