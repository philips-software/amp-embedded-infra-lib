#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/test_doubles/SesameMock.hpp"

namespace services
{
    class MethodSerializerMock
        : public MethodSerializer
    {
    public:
        MOCK_METHOD(bool, Serialize, (infra::SharedPtr<infra::StreamWriter> && writer), (override));
    };
}

class EchoOnSesameTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    void ReceiveMessage(infra::ConstByteRange data)
    {
        sesame.GetObserver().ReceivedMessage(reader.Emplace(data));
    }

    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> serializerFactory;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<services::SesameMock> sesame;
    services::EchoOnSesame echo{ sesame, serializerFactory, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echo };
    testing::StrictMock<services::ServiceStub> service{ echo };

    infra::SharedOptional<testing::StrictMock<services::MethodSerializerMock>> serializer;
    infra::SharedOptional<infra::ByteInputStreamReader> reader;
};

TEST_F(EchoOnSesameTest, invoke_service_proxy_method)
{
    sesame.GetObserver().Initialized();

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(sesame, RequestSendMessage(38));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));
    EXPECT_EQ((std::vector<uint8_t>{ 1, (1 << 3) | 2, 2, 8, 5 }), (std::vector<uint8_t>(writer.Processed().begin(), writer.Processed().end())));
}

TEST_F(EchoOnSesameTest, invoke_service_proxy_method_is_split_over_two_packets)
{
    sesame.GetObserver().Initialized();

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillRepeatedly(testing::Return(4));
    EXPECT_CALL(sesame, RequestSendMessage(4));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    EXPECT_CALL(sesame, RequestSendMessage(4));
    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 4);
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(limitedWriter));
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));
    EXPECT_EQ((std::vector<uint8_t>{ 1, (1 << 3) | 2, 2, 8, 5 }), (std::vector<uint8_t>(writer.Processed().begin(), writer.Processed().end())));
}

TEST_F(EchoOnSesameTest, invoke_service_proxy_method_without_parameters)
{
    sesame.GetObserver().Initialized();

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(sesame, RequestSendMessage(38));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));
    EXPECT_EQ((std::vector<uint8_t>{ 1, (3 << 3) | 2, 0 }), (std::vector<uint8_t>(writer.Processed().begin(), writer.Processed().end())));
}

TEST_F(EchoOnSesameTest, RequestSendMessage_is_delayed_until_Initialized)
{
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(sesame, RequestSendMessage(38));
    sesame.GetObserver().Initialized();
}

TEST_F(EchoOnSesameTest, partial_message_is_discarded_after_Initialize)
{
    sesame.GetObserver().Initialized();

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillRepeatedly(testing::Return(4));
    EXPECT_CALL(sesame, RequestSendMessage(4));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    EXPECT_CALL(sesame, RequestSendMessage(4));
    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 4);
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(limitedWriter));

    sesame.GetObserver().Initialized();

    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));
    EXPECT_EQ((std::vector<uint8_t>{ 1, (1 << 3) | 2, 2, 8 }), (std::vector<uint8_t>(writer.Processed().begin(), writer.Processed().end())));
}

TEST_F(EchoOnSesameTest, service_method_is_invoked)
{
    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    ReceiveMessage(infra::ConstructBin()({ 1, (1 << 3) | 2, 2, 8, 5 }).Range());
}

TEST_F(EchoOnSesameTest, service_method_bytes_is_invoked)
{
    EXPECT_CALL(service, MethodBytes(testing::_)).WillOnce(testing::Invoke([this](const infra::BoundedVector<uint8_t>& v)
        {
            const infra::BoundedVector<uint8_t>::WithMaxSize<4> bytes{ { 1, 2, 3, 4 } };
            EXPECT_EQ(bytes, v);
            service.MethodDone();
        }));
    ReceiveMessage(infra::ConstructBin()({ 1, (4 << 3) | 2, 6, 10, 4, 1, 2, 3, 4 }).Range());
}

TEST_F(EchoOnSesameTest, split_service_method_bytes_is_invoked)
{
    EXPECT_CALL(service, MethodBytes(testing::_)).WillOnce(testing::Invoke([this](const infra::BoundedVector<uint8_t>& v)
        {
            const infra::BoundedVector<uint8_t>::WithMaxSize<4> bytes{ { 1, 2, 3, 4 } };
            EXPECT_EQ(bytes, v);
            service.MethodDone();
        }));
    ReceiveMessage(infra::ConstructBin()({ 1, (4 << 3) | 2, 6, 10, 4, 1, 2 }).Range());
    ReceiveMessage(infra::ConstructBin()({ 3, 4 }).Range());
}

TEST_F(EchoOnSesameTest, service_method_without_parameter_is_invoked)
{
    EXPECT_CALL(service, MethodNoParameter()).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    ReceiveMessage(infra::ConstructBin()({ 1, (3 << 3) | 2, 0 }).Range());
}

TEST_F(EchoOnSesameTest, MessageFormatError_is_reported_when_message_is_not_a_LengthDelimited)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 0, 2 }).Range());
}

TEST_F(EchoOnSesameTest, MessageFormatError_is_reported_when_message_is_of_unknown_type)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 6 }).Range());
}

TEST_F(EchoOnSesameTest, MessageFormatError_is_reported_when_parameter_in_message_is_of_incorrect_type)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 5, 13, 5, 0, 0, 0 }).Range());
}

TEST_F(EchoOnSesameTest, ServiceNotFound_is_reported)
{
    EXPECT_CALL(errorPolicy, ServiceNotFound(2));
    ReceiveMessage(infra::ConstructBin()({ 2, 10, 2, 8, 5 }).Range());
}

TEST_F(EchoOnSesameTest, MethodNotFound_is_reported)
{
    EXPECT_CALL(errorPolicy, MethodNotFound(1, 2));
    ReceiveMessage(infra::ConstructBin()({ 1, 18, 2, 8, 5 }).Range());
}

TEST_F(EchoOnSesameTest, Reset_is_forwarded)
{
    EXPECT_CALL(sesame, Reset());
    echo.Reset();
}

TEST_F(EchoOnSesameTest, Reset_releases_reader)
{
    EXPECT_CALL(service, Method(5));

    ReceiveMessage(infra::ConstructBin()({ 1, 10, 2, 8, 5, 1, 10 }).Range());

    ASSERT_FALSE(reader.Allocatable());

    EXPECT_CALL(sesame, Reset());
    echo.Reset();

    EXPECT_TRUE(reader.Allocatable());
}

TEST_F(EchoOnSesameTest, Reset_forgets_send_requests)
{
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    EXPECT_CALL(sesame, Reset());
    echo.Reset();
    sesame.GetObserver().Initialized();

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(sesame, RequestSendMessage(38));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));
    EXPECT_EQ((std::vector<uint8_t>{ 1, (1 << 3) | 2, 2, 8, 5 }), (std::vector<uint8_t>(writer.Processed().begin(), writer.Processed().end())));
}

TEST_F(EchoOnSesameTest, after_Reset_RequestSendMessage_is_delayed_until_Initialized)
{
    // build
    sesame.GetObserver().Initialized();

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(sesame, RequestSendMessage(38));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    sesame.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));

    // operate
    EXPECT_CALL(sesame, Reset());
    echo.Reset();

    // check
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    EXPECT_CALL(sesame, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(sesame, RequestSendMessage(38));
    sesame.GetObserver().Initialized();
}
