#ifndef SERVICES_CONSOLE_SERVICE_HPP
#define SERVICES_CONSOLE_SERVICE_HPP

#include "infra/stream/StringInputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Echo.hpp"
#include "services/echo_console/Console.hpp"
#include <cstdint>
#include <cstdlib>

namespace services
{
    class GenericMethodDeserializer
        : public MethodDeserializer
    {
    public:
        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;
    };

    class ConsoleServiceMethodExecute
    {
    public:
        virtual infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy);

    private:
        infra::SharedOptional<GenericMethodDeserializer> methodDeserializer;
    };

    class ConsoleService
        : public Service
    {
    public:
        ConsoleService(Echo& echo, uint32_t serviceId, ConsoleServiceMethodExecute& methodExecute);

        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy) override;

    private:
        ConsoleServiceMethodExecute& methodExecute;
    };

    class GenericMethodSerializer
        : public MethodSerializer
    {
    public:
        GenericMethodSerializer(infra::SharedPtr<infra::StringInputStream>& inputStream);

        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        infra::SharedPtr<infra::StringInputStream> inputStream;
    };

    class ConsoleServiceProxy
        : public ServiceProxy
    {
    public:
        ConsoleServiceProxy(Echo& echo, uint32_t maxMessageSize = 1000);

        void SendMessage(infra::SharedPtr<infra::StringInputStream>& inputStream);

    private:
        infra::SharedOptional<GenericMethodSerializer> methodSerializer;
    };
}
#endif // CONSOLESERVICE_HPP
