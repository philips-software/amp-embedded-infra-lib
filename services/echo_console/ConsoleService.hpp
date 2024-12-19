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
        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override
        {
            int a = 3;
            a++;
        }

        void ExecuteMethod() override
        {
            int a = 3;
            a++;
        }

        bool Failed() const override
        {
            return false;
        }
    };

    class ConsoleServiceMethodExecute
    {
    public:
        ConsoleServiceMethodExecute(application::Console& console)
            : console(console)
        {}

        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy)
        {
            return methodDeserializer.Emplace();
        }

    private:
        application::Console& console;
        infra::SharedOptional<GenericMethodDeserializer> methodDeserializer;
    };

    class ConsoleService
        : public Service
    {
    public:
        ConsoleService(Echo& echo, uint32_t serviceId, ConsoleServiceMethodExecute& methodExecute)
            : Service(echo, serviceId)
            , methodExecute(methodExecute)
        {}

        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy) override
        {
            return methodExecute.StartMethod(serviceId, methodId, size, errorPolicy);
        }

    private:
        ConsoleServiceMethodExecute& methodExecute;
    };

    class GenericMethodSerializer
        : public MethodSerializer
    {
    public:
        GenericMethodSerializer(infra::SharedPtr<infra::StringInputStream>& inputStream)
            : inputStream(inputStream)
        {}

        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override
        {
            infra::DataOutputStream::WithErrorPolicy stream(*writer, infra::softFail);

            stream << inputStream->ContiguousRange(stream.Available());

            auto partlySent = stream.Failed() || inputStream->Available();
            writer = nullptr;

            return partlySent;
        }

    private:
        infra::SharedPtr<infra::StringInputStream> inputStream;
    };

    class ConsoleServiceProxy
        : public ServiceProxy
    {
    public:
        ConsoleServiceProxy(Echo& echo, uint32_t maxMessageSize = 1000)
            : ServiceProxy(echo, maxMessageSize)
        {}

        void SendMessage(infra::SharedPtr<infra::StringInputStream>& inputStream)
        {
            auto serializer = methodSerializer.Emplace(inputStream);
            SetSerializer(serializer);
        }

    private:
        infra::SharedOptional<GenericMethodSerializer> methodSerializer;
    };
}
#endif // CONSOLESERVICE_HPP
