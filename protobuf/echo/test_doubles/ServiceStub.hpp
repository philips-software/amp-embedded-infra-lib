#ifndef PROTOBUF_TEST_SERVICE_STUB_HPP
#define PROTOBUF_TEST_SERVICE_STUB_HPP

#include "protobuf/echo/Echo.hpp"
#include "gmock/gmock.h"

namespace services
{
    struct Message
    {
    public:
        static const uint32_t numberOfFields = 1;
        template<std::size_t fieldIndex>
        using ProtoType = ProtoUInt32;
        template<std::size_t fieldIndex>
        using Type = uint32_t;
        template<std::size_t fieldIndex>
        static const uint32_t fieldNumber = 1;

    public:
        Message() = default;
        Message(uint32_t value);

        void Serialize(infra::ProtoFormatter& formatter) const;

        uint32_t& Get(std::integral_constant<uint32_t, 0>);
        const uint32_t& Get(std::integral_constant<uint32_t, 0>) const;

    public:
        uint32_t value = 0;
    };

    class ServiceStub
        : public services::Service
    {
    public:
        ServiceStub(Echo& echo, MethodSerializerFactory& serializerFactory);

        bool AcceptsService(uint32_t id) const override;

        MOCK_METHOD(void, Method, (uint32_t value));
        MOCK_METHOD(void, MethodNoParameter, ());

    protected:
        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy) override;

    public:
        static const uint32_t serviceId = 1;
        static const uint32_t idMethod = 1;
        static const uint32_t idMethodNoParameter = 3;
        static const uint32_t maxMessageSize = 18;

    public:
        using MethodTypeList = infra::List<Message, EmptyMessage>;

    private:
        MethodSerializerFactory& serializerFactory;
    };

    class ServiceStubProxy
        : public services::ServiceProxy
    {
    public:
        ServiceStubProxy(services::Echo& echo, MethodSerializerFactory& serializerFactory);

    public:
        void Method(uint32_t value);
        void MethodNoParameter();

    public:
        static constexpr uint32_t serviceId = 1;
        static constexpr uint32_t idMethod = 1;
        static const uint32_t idMethodNoParameter = 3;
        static constexpr uint32_t maxMessageSize = 18;

    public:
        using MethodTypeList = infra::List<Message, EmptyMessage>;

    private:
        MethodSerializerFactory& serializerFactory;
    };
}

#endif
