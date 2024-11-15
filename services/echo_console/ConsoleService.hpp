#ifndef SERVICES_CONSOLE_SERVICE_HPP
#define SERVICES_CONSOLE_SERVICE_HPP

#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Echo.hpp"
#include "services/echo_console/Console.hpp"
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
}
#endif // CONSOLESERVICE_HPP
