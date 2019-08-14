#ifndef SERVICES_STOPPABLE_MOCK_HPP
#define SERVICES_STOPPABLE_MOCK_HPP

#include "gmock/gmock.h"
#include "services/util/Stoppable.hpp"

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
