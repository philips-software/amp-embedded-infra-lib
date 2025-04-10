#ifndef SERVICES_CONSOLE_SERVICE_HPP
#define SERVICES_CONSOLE_SERVICE_HPP

#include "infra/stream/ByteInputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Echo.hpp"
#include <cstdint>
#include <cstdlib>

namespace services
{
    class GenericMethodDeserializerFactory
    {
    public:
        virtual infra::SharedPtr<MethodDeserializer> MakeDeserializer(uint32_t serviceId, uint32_t methodId, uint32_t size) = 0;
    };

    class ConsoleService
        : public Service
    {
    public:
        ConsoleService(Echo& echo, uint32_t serviceId, GenericMethodDeserializerFactory& deserializerFactory);

        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy) override;

    private:
        GenericMethodDeserializerFactory& methodDeserializerFactory;
    };

    class GenericMethodSerializer
        : public MethodSerializer
    {
    public:
        GenericMethodSerializer(infra::SharedPtr<infra::ByteInputStream>& inputStream);

        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        infra::SharedPtr<infra::ByteInputStream> inputStream;
    };

    class ConsoleServiceProxy
        : public ServiceProxy
    {
    public:
        ConsoleServiceProxy(Echo& echo, uint32_t maxMessageSize = 1000);

        void SendMessage(infra::SharedPtr<infra::ByteInputStream>& inputStream);

    private:
        infra::SharedOptional<GenericMethodSerializer> methodSerializer;
    };
}
#endif // CONSOLESERVICE_HPP
