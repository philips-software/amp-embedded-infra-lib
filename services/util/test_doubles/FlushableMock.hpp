#ifndef SERVICES_UTIL_TEST_DOUBLES_FLUSHABLEMOCK_HPP
#define SERVICES_UTIL_TEST_DOUBLES_FLUSHABLEMOCK_HPP

#include "services/util/Flushable.hpp"
#include "gmock/gmock.h"

namespace services
{
    class FlushableMock
        : public services::Flushable
    {
    public:
        MOCK_METHOD(void, Flush, (), (override));
    };
}

#endif // SERVICES_UTIL_TEST_DOUBLES_FLUSHABLEMOCK_HPP
