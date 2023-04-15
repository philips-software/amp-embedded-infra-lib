#ifndef PROTOBUF_TEST_SERVICE_STUB_HPP
#define PROTOBUF_TEST_SERVICE_STUB_HPP

#include "gmock/gmock.h"
#include "protobuf/echo/Echo.hpp"

namespace services
{
    class TestService1
        : public services::Service
    {
    public:
        using services::Service::Service;

    public:
        virtual bool AcceptsService(uint32_t id) const = 0;
        virtual void Method(uint32_t value) = 0;

    protected:
        virtual void Handle(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, services::EchoErrorPolicy& errorPolicy) override;

    public:
        static const uint32_t serviceId = 1;
        static const uint32_t idMethod = 1;
        static const uint32_t maxMessageSize = 18;
    };

    class ServiceStub
        : public TestService1
    {
    public:
        using TestService1::TestService1;

        MOCK_CONST_METHOD1(AcceptsService, bool(uint32_t id));
        MOCK_METHOD1(Method, void(uint32_t value));
    };

    class ServiceStubProxy
        : public services::ServiceProxy
    {
    public:
        ServiceStubProxy(services::Echo& echo);

    public:
        void Method(uint32_t value);

    public:
        static const uint32_t serviceId = 1;
        static const uint32_t idMethod = 1;
        static const uint32_t maxMessageSize = 18;
    };
}

#endif
