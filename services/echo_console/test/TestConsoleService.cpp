#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/InputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Echo.hpp"
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

class ServiceMock
    : public services::Service
{
public:
    using services::Service::Service;

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

    const uint8_t serviceId = 1;
    const uint8_t methodId = 2;

    testing::StrictMock<MethodDeserializerMock> deserializer;
    testing::StrictMock<ServiceMock> service{ echo, 1 };
};

TEST_F(ConsoleServiceProxyTest, send_single_piece_message)
{
    std::array<uint8_t, 5> message{ serviceId, (uint8_t)((methodId << 3) | 2), 2, 8, 5 };

    infra::NotifyingSharedOptional<infra::ByteInputStream> reader;
    auto readerPtr = reader.Emplace(message, infra::noFail);

    EXPECT_CALL(service, StartMethod(serviceId, methodId, testing::_, testing::_)).Times(1).WillRepeatedly(testing::InvokeWithoutArgs([this]
        {
            return infra::UnOwnedSharedPtr(deserializer);
        }));

    EXPECT_CALL(deserializer, MethodContents(testing::_)).Times(1).WillRepeatedly(testing::Invoke([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            reader->ExtractContiguousRange(reader->Available());
        }));

    EXPECT_CALL(deserializer, ExecuteMethod()).Times(1).WillRepeatedly(testing::Invoke([this]()
        {
            echo.ServiceDone();
        }));

    EXPECT_CALL(deserializer, Failed()).WillRepeatedly(testing::Return(false));

    consoleServiceProxy.RequestSend([this, &readerPtr]()
        {
            consoleServiceProxy.SendMessage(readerPtr);
        });
}

TEST_F(ConsoleServiceProxyTest, send_multi_part_message)
{
    std::vector<uint8_t> sequence{ serviceId, (uint8_t)((methodId << 3) | 2), 2, 8, 5 };
    std::vector<uint8_t> message;

    auto const methodCount = 5;

    for (auto i = 0; i < methodCount; ++i)
    {
        message.insert(message.end(), sequence.begin(), sequence.end());
    }

    infra::NotifyingSharedOptional<infra::ByteInputStream> reader;
    auto readerPtr = reader.Emplace(message, infra::noFail);

    services::ConsoleServiceProxy consoleServiceProxySmallMessageSize{ echo, 3 };

    EXPECT_CALL(service, StartMethod(serviceId, methodId, testing::_, testing::_)).Times(methodCount).WillRepeatedly(testing::InvokeWithoutArgs([this]
        {
            return infra::UnOwnedSharedPtr(deserializer);
        }));

    EXPECT_CALL(deserializer, MethodContents(testing::_)).Times(methodCount+1).WillRepeatedly(testing::Invoke([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            reader->ExtractContiguousRange(reader->Available());
        }));

    EXPECT_CALL(deserializer, ExecuteMethod()).Times(methodCount).WillRepeatedly(testing::Invoke([this]()
        {
            echo.ServiceDone();
        }));

    EXPECT_CALL(deserializer, Failed()).WillRepeatedly(testing::Return(false));

    consoleServiceProxySmallMessageSize.RequestSend([&consoleServiceProxySmallMessageSize, &readerPtr]()
        {
            consoleServiceProxySmallMessageSize.SendMessage(readerPtr);
        });
}
