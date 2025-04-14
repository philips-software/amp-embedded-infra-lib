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
