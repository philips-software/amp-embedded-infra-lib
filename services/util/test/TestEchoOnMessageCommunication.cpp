#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoOnMessageCommunication.hpp"
#include "services/util/test_doubles/MessageCommunicationMock.hpp"

namespace services
{
    class MethodSerializerMock
        : public MethodSerializer
    {
    public:
        MOCK_METHOD(bool, Serialize, (infra::SharedPtr<infra::StreamWriter> && writer), (override));
    };
}

class EchoOnMessageCommunicationTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    void ReceiveMessage(infra::ConstByteRange data)
    {
        infra::ByteInputStreamReader reader{ data };
        messageCommunication.GetObserver().ReceivedMessage(infra::UnOwnedSharedPtr(reader));
    }

    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<services::MessageCommunicationMock> messageCommunication;
    services::EchoOnMessageCommunication echo{ messageCommunication, errorPolicy };

    services::MethodDeserializerFactory::ForMessage<services::Message> deserializerFactory;
    services::ServiceStubProxy serviceProxy{ echo };
    testing::StrictMock<services::ServiceStub> service{ echo, deserializerFactory };

    infra::SharedOptional<testing::StrictMock<services::MethodSerializerMock>> serializer;
};

TEST_F(EchoOnMessageCommunicationTest, invoke_service_proxy_method)
{
    EXPECT_CALL(messageCommunication, MaxSendMessageSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(messageCommunication, RequestSendMessage(18));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    messageCommunication.GetObserver().SendMessageStreamAvailable(infra::UnOwnedSharedPtr(writer));

    EXPECT_EQ((std::vector<uint8_t>{ 1, 10, 2, 8, 5 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 5)));
}

TEST_F(EchoOnMessageCommunicationTest, service_method_is_invoked)
{
    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 2, 8, 5 }).Range());
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
    ReceiveMessage(infra::ConstructBin()({ 1, 10, 5, 13, 5, 0, 0, 0 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, ServiceNotFound_is_reported)
{
    EXPECT_CALL(errorPolicy, ServiceNotFound(2));
    ReceiveMessage(infra::ConstructBin()({ 2, 10, 2, 8, 5 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, MethodNotFound_is_reported)
{
    EXPECT_CALL(errorPolicy, MethodNotFound(1, 2));
    ReceiveMessage(infra::ConstructBin()({ 1, 18, 2, 8, 5 }).Range());
}
