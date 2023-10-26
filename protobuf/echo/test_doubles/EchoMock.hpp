#ifndef PROTOBUF_TEST_HELPER_ECHO_HPP
#define PROTOBUF_TEST_HELPER_ECHO_HPP

#include "protobuf/echo/Echo.hpp"
#include "gmock/gmock.h"

namespace services
{
    class EchoMock
        : public services::Echo
    {
    public:
        MOCK_METHOD(void, RequestSend, (ServiceProxy & serviceProxy), (override));
        MOCK_METHOD(void, ServiceDone, (), (override));
    };

    class EchoErrorPolicyMock
        : public services::EchoErrorPolicy
    {
    public:
        MOCK_METHOD(void, MessageFormatError, (), (const override));
        MOCK_METHOD(void, ServiceNotFound, (uint32_t serviceId), (const override));
        MOCK_METHOD(void, MethodNotFound, (uint32_t serviceId, uint32_t methodId), (const override));
    };
}

#endif
