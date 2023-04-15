#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/test/ServiceStub.hpp"
#include "protobuf/echo/test_helper/EchoMock.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"

class EchoOnMessageCommunicationTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    EchoOnMessageCommunicationTest()
    {
        EXPECT_CALL(service, AcceptsService(1)).WillRepeatedly(testing::Return(true));
    }

    void ReceiveMessage(infra::ConstByteRange data)
    {
        infra::ByteInputStreamReader reader{ data };
        messageCommunication.GetObserver().ReceivedMessage(infra::UnOwnedSharedPtr(reader));
    }

    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<services::MessageCommunicationMock> messageCommunication;
    services::EchoOnMessageCommunication echo{ messageCommunication, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echo };
    testing::StrictMock<services::ServiceStub> service{ echo };
};

TEST_F(EchoOnMessageCommunicationTest, invoke_service_proxy_method)
{
    testing::StrictMock<infra::MockCallback<void()>> onGranted;
    EXPECT_CALL(messageCommunication, RequestSendMessage(18));
    serviceProxy.RequestSend([&onGranted]()
        { onGranted.callback(); });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    EXPECT_CALL(onGranted, callback());
    messageCommunication.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));

    serviceProxy.Method(5);
    EXPECT_EQ((std::vector<uint8_t>{ 1, 10, 2, 8, 5 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 5)));
}

TEST_F(EchoOnMessageCommunicationTest, service_method_is_invoked)
{
    EXPECT_CALL(service, Method(5));
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 2, 8, 5 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, on_partial_MessageFormatError_is_reported)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 2, 8 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, MessageFormatError_is_reported_when_message_is_not_a_LengthDelimited)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 0, 2 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, MessageFormatError_is_reported_when_message_is_of_unknown_type)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 6 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, MessageFormatError_is_reported_when_parameter_in_message_is_of_incorrect_type)
{
    EXPECT_CALL(errorPolicy, MessageFormatError());
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 2, 13, 5, 0, 0, 0 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, ServiceNotFound_is_reported)
{
    EXPECT_CALL(service, AcceptsService(2)).WillOnce(testing::Return(false));
    EXPECT_CALL(errorPolicy, ServiceNotFound(2));
    ReceiveMessage(infra::ConstructBin()({ 2, 10, 2, 8, 5 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, MethodNotFound_is_reported)
{
    EXPECT_CALL(errorPolicy, MethodNotFound(1, 2));
    ReceiveMessage(infra::ConstructBin()({ 1, 18, 2, 8, 5 }).Range());
}
