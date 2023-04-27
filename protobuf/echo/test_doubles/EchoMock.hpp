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
        MOCK_METHOD1(RequestSend, void(ServiceProxy& serviceProxy));
        MOCK_METHOD0(SendStreamWriter, infra::StreamWriter&());
        MOCK_METHOD0(Send, void());
        MOCK_METHOD1(ServiceDone, void(Service& service));
    };

    class EchoErrorPolicyMock
        : public services::EchoErrorPolicy
    {
    public:
        MOCK_METHOD0(MessageFormatError, void());
        MOCK_METHOD1(ServiceNotFound, void(uint32_t serviceId));
        MOCK_METHOD2(MethodNotFound, void(uint32_t serviceId, uint32_t methodId));
    };

    class MessageCommunicationMock
        : public services::MessageCommunication
    {
    public:
        MOCK_METHOD1(RequestSendMessage, void(uint16_t size));
        MOCK_CONST_METHOD0(MaxSendMessageSize, std::size_t());
    };
}

#endif
