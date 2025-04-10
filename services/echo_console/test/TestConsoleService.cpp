#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/InputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Serialization.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "services/echo_console/ConsoleService.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <array>
#include <cstdint>

class MethodDeserializerMock
    : public services::MethodDeserializer
{
public:
    MOCK_METHOD(void, MethodContents, (infra::SharedPtr<infra::StreamReaderWithRewinding> && reader), (override));
    MOCK_METHOD(void, ExecuteMethod, (), (override));
    MOCK_METHOD(bool, Failed, (), (const, override));
};

class GenericMethodDeserializerFactoryStub
    : public services::GenericMethodDeserializerFactory
{
public:
    GenericMethodDeserializerFactoryStub(infra::Function<void()> serviceDone)
        : serviceDone(serviceDone)
    {}

    infra::SharedPtr<services::MethodDeserializer> MakeDeserializer(uint32_t serviceId, uint32_t methodId, uint32_t size) override
    {
        auto ptr = deserializer.Emplace();

        EXPECT_EQ(serviceId, 1);
        EXPECT_EQ(methodId, 2);

        EXPECT_CALL(*deserializer, MethodContents(testing::_)).WillOnce(testing::Invoke([size](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                reader->ExtractContiguousRange(size);
            }));

        EXPECT_CALL(*deserializer, ExecuteMethod()).WillOnce(testing::Invoke([this]()
            {
                serviceDone();
            }));

        EXPECT_CALL(*deserializer, Failed()).WillOnce(testing::Return(false));

        return ptr;
    }

    infra::SharedOptional<testing::StrictMock<MethodDeserializerMock>> deserializer;
    infra::Function<void()> serviceDone;
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
    GenericMethodDeserializerFactoryStub deserializerFactory{ [this]()
        {
            echo.ServiceDone();
        } };
    services::ConsoleService service{ echo, 1, deserializerFactory };
};

TEST_F(ConsoleServiceProxyTest, construction)
{}

TEST_F(ConsoleServiceProxyTest, send)
{
    const auto serviceId = 1;
    const auto methodId = 2;

    static infra::StdVectorOutputStream::WithStorage stream;
    stream << infra::ConstructBin()({ serviceId, (methodId << 3) | 2, 2, 8, 5 }).Range();

    infra::NotifyingSharedOptional<infra::ByteInputStream> reader;
    auto readerPtr = reader.Emplace(stream.Storage(), infra::noFail);

    consoleServiceProxy.RequestSend([this, &readerPtr]()
        {
            consoleServiceProxy.SendMessage(readerPtr);
        });
}
