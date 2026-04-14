#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "protobuf/echo/EchoOnStreams.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "gmock/gmock.h"

namespace services
{
    class EchoOnStreamsMock
        : public EchoOnStreams
    {
    public:
        using EchoOnStreams::EchoOnStreams;

        MOCK_METHOD(infra::SharedPtr<MethodSerializer>, GrantSend, (ServiceProxy & proxy), (override));
        MOCK_METHOD(infra::SharedPtr<MethodDeserializer>, StartingMethod, (uint32_t serviceId, uint32_t methodId, infra::SharedPtr<MethodDeserializer>&& deserializer), (override));
        MOCK_METHOD(void, RequestSendStream, (std::size_t size), (override));
        MOCK_METHOD(void, MethodContents, (infra::SharedPtr<infra::StreamReaderWithRewinding> && reader), (override));
        MOCK_METHOD(void, ReleaseDeserializer, (), (override));

        using EchoOnStreams::DataReceived;
        using EchoOnStreams::Initialized;
        using EchoOnStreams::ReleaseReader;
        using EchoOnStreams::SendStreamAvailable;

        infra::SharedPtr<MethodSerializer> InheritedGrantSend(ServiceProxy& proxy)
        {
            return EchoOnStreams::GrantSend(proxy);
        }
    };
}

class EchoOnStreamsTest
    : public testing::Test
{
public:
    infra::SharedOptional<infra::ByteInputStreamReader> reader;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> serializerFactory;
    testing::StrictMock<services::EchoOnStreamsMock> echo{ serializerFactory, errorPolicy };
    testing::StrictMock<services::ServiceStub> service{ echo };
    services::ServiceStubProxy serviceProxy{ echo };

    std::vector<uint8_t> data;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
};

TEST_F(EchoOnStreamsTest, send_method_without_parameter)
{
    EXPECT_CALL(echo, RequestSendStream(services::ServiceStubProxy::maxMessageSize + 2 * 10));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    EXPECT_CALL(echo, GrantSend(testing::Ref(serviceProxy))).WillOnce(testing::Invoke([this](services::ServiceProxy& serviceProxy)
        {
            return echo.InheritedGrantSend(serviceProxy);
        }));
    echo.SendStreamAvailable(writer.Emplace(data));
    EXPECT_EQ((std::vector<uint8_t>{ 1, 26, 0 }), data);
}

TEST_F(EchoOnStreamsTest, ReleaseReader_with_data_in_buffer)
{
    std::array<uint8_t, 64> data{ 1, (1 << 3) | 2, 64 };
    EXPECT_CALL(echo, StartingMethod(1, 1, testing::_)).WillOnce(testing::Invoke([](uint32_t serviceId, uint32_t methodId, infra::SharedPtr<services::MethodDeserializer>&& deserializer)
        {
            return std::move(deserializer);
        }));
    EXPECT_CALL(echo, MethodContents(testing::_));
    echo.DataReceived(reader.Emplace(infra::MakeRange(data)));

    echo.ReleaseReader();
}

TEST_F(EchoOnStreamsTest, request_send_while_already_awaiting_grant_aborts)
{
    EXPECT_CALL(echo, RequestSendStream(testing::_));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    EXPECT_DEATH(
        {
            serviceProxy.RequestSend([this]()
                {
                    serviceProxy.MethodNoParameter();
                });
        },
        "");
}

class ServiceProxyCancelTest
    : public testing::Test
{
public:
    testing::StrictMock<services::EchoMock> echo;
    services::ServiceStubProxy serviceProxy{ echo };
};

TEST_F(ServiceProxyCancelTest, cancel_clears_pending_grant_and_notifies_echo)
{
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    serviceProxy.RequestSend([]() {});

    EXPECT_CALL(echo, CancelRequestSend(testing::Ref(serviceProxy)));
    serviceProxy.CancelRequestSend();
}

TEST_F(ServiceProxyCancelTest, cancel_without_pending_request_is_a_noop)
{
    serviceProxy.CancelRequestSend(); // no expectations set — must not call echo at all
}

TEST_F(ServiceProxyCancelTest, cancel_and_retry_request_send_succeeds)
{
    // First request
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    serviceProxy.RequestSend([]() {});

    // Cancel it
    EXPECT_CALL(echo, CancelRequestSend(testing::Ref(serviceProxy)));
    serviceProxy.CancelRequestSend();

    // Second request on the same proxy must not assert
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    bool granted = false;
    serviceProxy.RequestSend([&granted]()
        {
            granted = true;
        });

    // Grant it to verify the new callback fires
    serviceProxy.GrantSend();
    EXPECT_TRUE(granted);
}
