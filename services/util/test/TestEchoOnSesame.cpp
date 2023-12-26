#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
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
        infra::ByteInputStreamReader reader{ data };
        sesame.GetObserver().ReceivedMessage(infra::UnOwnedSharedPtr(reader));
    }

    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> serializerFactory;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<services::SesameMock> sesame;
    services::EchoOnSesame echo{ sesame, serializerFactory, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echo };
    testing::StrictMock<services::ServiceStub> service{ echo };

    infra::SharedOptional<testing::StrictMock<services::MethodSerializerMock>> serializer;
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

    EXPECT_EQ((std::vector<uint8_t>{ 1, (1 << 3) | 2, 2, 8, 5 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 5)));
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

    EXPECT_EQ((std::vector<uint8_t>{ 1, (3 << 3) | 2, 0 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 3)));
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

TEST_F(EchoOnSesameTest, service_method_is_invoked)
{
    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 2, 8, 5 }).Range());
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