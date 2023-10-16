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
        MOCK_METHOD0(MessageFormatError, void());
        MOCK_METHOD1(ServiceNotFound, void(uint32_t serviceId));
        MOCK_METHOD2(MethodNotFound, void(uint32_t serviceId, uint32_t methodId));
    };
}

#endif
