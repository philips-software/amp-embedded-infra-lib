#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/InputStream.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/Serialization.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/echo_console/ConsoleService.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <array>
#include <cstdint>
#include <vector>

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
    services::MethodSerializerFactory::OnHeap serializerFactory;

    application::EchoSingleLoopback echo{ serializerFactory };
    services::EchoConsoleServiceProxy consoleServiceProxy{ echo };

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

    services::EchoConsoleServiceProxy consoleServiceProxySmallMessageSize{ echo, 3 };

    EXPECT_CALL(service, StartMethod(serviceId, methodId, testing::_, testing::_)).Times(methodCount).WillRepeatedly(testing::InvokeWithoutArgs([this]
        {
            return infra::UnOwnedSharedPtr(deserializer);
        }));

    EXPECT_CALL(deserializer, MethodContents(testing::_)).Times(methodCount + 1).WillRepeatedly(testing::Invoke([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
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

class EchoConsoleMethodExecutionMock
    : public services::EchoConsoleMethodExecution
{
public:
    MOCK_METHOD(void, ExecuteMethod, (infra::StreamReader & data), (override));
};

class ConsoleServiceTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::OnHeap serializerFactory;

    application::EchoSingleLoopback echo{ serializerFactory };

    uint32_t acceptExceptServiceId = 100;
    testing::StrictMock<EchoConsoleMethodExecutionMock> methodExecution;
    services::EchoConsoleService consoleService{ echo, acceptExceptServiceId, methodExecution };
    services::ServiceStubProxy serviceStubProxy{ echo };
};

TEST_F(ConsoleServiceTest, accept_service)
{
    EXPECT_TRUE(consoleService.AcceptsService(1));
    EXPECT_FALSE(consoleService.AcceptsService(acceptExceptServiceId));
}

TEST_F(ConsoleServiceTest, start_method_without_args)
{
    EXPECT_CALL(methodExecution, ExecuteMethod(testing::_)).Times(1).WillOnce(testing::Invoke([this](infra::StreamReader& reader)
        {
            infra::DataInputStream::WithErrorPolicy inputStream(reader);
            infra::ProtoParser parser(inputStream);

            auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
            EXPECT_EQ(serviceId, services::ServiceStubProxy::defaultServiceId);

            auto [value, methodId] = parser.GetField();
            EXPECT_EQ(methodId, services::ServiceStubProxy::idMethodNoParameter);

            echo.ServiceDone();
        }));

    serviceStubProxy.RequestSend([this]()
        {
            serviceStubProxy.MethodNoParameter();
        });
}

TEST_F(ConsoleServiceTest, start_method)
{
    EXPECT_CALL(methodExecution, ExecuteMethod(testing::_)).Times(1).WillOnce(testing::Invoke([this](infra::StreamReader& reader)
        {
            infra::DataInputStream::WithErrorPolicy inputStream(reader);
            infra::ProtoParser parser(inputStream);

            auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
            EXPECT_EQ(serviceId, services::ServiceStubProxy::defaultServiceId);

            auto [value, methodId] = parser.GetField();
            EXPECT_EQ(methodId, services::ServiceStubProxy::idMethod);

            auto parameters = value.Get<infra::ProtoLengthDelimited>().Parser();
            auto [valueP, idP] = parser.GetField();
            EXPECT_EQ(idP, 1);
            EXPECT_EQ(valueP.Get<uint64_t>(), 123);

            echo.ServiceDone();
        }));

    serviceStubProxy.RequestSend([this]()
        {
            serviceStubProxy.Method(123);
        });
}
