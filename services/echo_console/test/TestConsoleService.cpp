#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "services/echo_console/ConsoleService.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class ConsoleServiceMethodExecuteMock
    : public services::ConsoleServiceMethodExecute
{
public:
    MOCK_METHOD(infra::SharedPtr<services::MethodDeserializer>, StartMethod, (uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy), (override));
};

class ConsoleServiceProxyTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery,
        service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy,
        service_discovery::ServiceDiscoveryResponseProxy>
        serializerFactory;

    application::EchoSingleLoopback echo{ serializerFactory };
    services::ConsoleServiceProxy consoleServiceProxy{ echo };
    testing::StrictMock<ConsoleServiceMethodExecuteMock> methodExecute;
    services::ConsoleService service{ echo, 1, methodExecute };
};

TEST_F(ConsoleServiceProxyTest, construction)
{}

TEST_F(ConsoleServiceProxyTest, send)
{
    // const auto serviceId = 1;
    // const auto methodId = 2;

    infra::StdVectorOutputStream::WithStorage stream;
    stream << infra::ConstructBin()({ 1, (1 << 3) | 2, 2, 8, 5 }).Range();

    // infra::ProtoFormatter formatter(stream);

    // formatter.PutVarInt(serviceId);
    // {
    //     auto subFormatter = formatter.LengthDelimitedFormatter(methodId);
    //     // methodInvocation.EncodeParameters(method.parameter, line.size(), formatter); //TODO
    // }

    infra::NotifyingSharedOptional<infra::StringInputStream> reader;
    auto readerPtr = reader.Emplace(infra::ByteRangeAsStdString(infra::MakeRange(stream.Storage())), infra::softFail);

    consoleServiceProxy.RequestSend([this, &readerPtr]()
        {
            consoleServiceProxy.SendMessage(readerPtr);
        });
}
