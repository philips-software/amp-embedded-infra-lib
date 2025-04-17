#ifndef SERVICES_CONSOLE_SERVICE_HPP
#define SERVICES_CONSOLE_SERVICE_HPP

#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/InputStream.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Echo.hpp"
#include <cstdint>
#include <cstdlib>

namespace services
{
    class EchoConsoleMethodExecution
    {
    public:
        virtual void ExecuteMethod(infra::StreamReader& data) = 0;
    };

    class EchoConsoleMethodDeserializer
        : public MethodDeserializer
    {
    public:
        explicit EchoConsoleMethodDeserializer(uint32_t serviceId, uint32_t methodId, uint32_t size, EchoConsoleMethodExecution& methodExecution);

        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        const uint32_t serviceId;
        const uint32_t methodId;
        const uint32_t size;
        EchoConsoleMethodExecution& methodExecution;
        infra::StdVectorInputStream::WithStorage stream;
        bool sentHeader = false;
    };

    class EchoConsoleService
        : public Service
        , public EchoConsoleMethodExecution
    {
    public:
        EchoConsoleService(Echo& echo, uint32_t acceptExcept, EchoConsoleMethodExecution& methodExecution);

        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy) override;
        bool AcceptsService(uint32_t id) const override;

        void ExecuteMethod(infra::StreamReader& data) override;

    private:
        uint32_t acceptExcept;
        infra::SharedOptional<EchoConsoleMethodDeserializer> methodDeserializer;
        EchoConsoleMethodExecution& methodExecution;
    };

    class EchoConsoleMethodSerializer
        : public MethodSerializer
    {
    public:
        EchoConsoleMethodSerializer(infra::SharedPtr<infra::ByteInputStream>& inputStream);

        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        infra::SharedPtr<infra::ByteInputStream> inputStream;
    };

    class EchoConsoleServiceProxy
        : public ServiceProxy
    {
    public:
        EchoConsoleServiceProxy(Echo& echo, uint32_t maxMessageSize = 1000);

        void SendMessage(infra::SharedPtr<infra::ByteInputStream>& inputStream);

    private:
        infra::SharedOptional<EchoConsoleMethodSerializer> methodSerializer;
    };
}
#endif // CONSOLESERVICE_HPP
