#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/Echo.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class EchoErrorPolicyMock
        : public services::EchoErrorPolicy
    {
    public:
        MOCK_METHOD0(MessageFormatError, void());
        MOCK_METHOD1(ServiceNotFound, void(uint32_t serviceId));
        MOCK_METHOD2(MethodNotFound, void(uint32_t serviceId, uint32_t methodId));
    };

    class TestService1
        : public services::Service
    {
    public:
        TestService1(services::Echo& echo);

    public:
        virtual void Method(uint32_t value) = 0;
        virtual void Handle(uint32_t methodId, infra::ProtoLengthDelimited& contents, services::EchoErrorPolicy& errorPolicy) override;

    public:
        static const uint32_t serviceId = 1;
        static const uint32_t idMethod = 1;
        static const uint32_t maxMessageSize = 18;
    };

    class TestService1Mock
        : public TestService1
    {
    public:
        using TestService1::TestService1;

        MOCK_METHOD1(Method, void(uint32_t value));
    };

    class TestService1Proxy
        : public services::ServiceProxy
    {
    public:
        TestService1Proxy(services::Echo& echo);

    public:
        void Method(uint32_t value);

    public:
        static const uint32_t serviceId = 1;
        static const uint32_t idMethod = 1;
        static const uint32_t maxMessageSize = 18;
    };

      TestService1::TestService1(services::Echo& echo)
        : services::Service(echo, serviceId)
    {}

    void TestService1::Method(uint32_t value)
    {}

    void TestService1::Handle(uint32_t methodId, infra::ProtoLengthDelimited& contents, services::EchoErrorPolicy& errorPolicy)
    {
        infra::ProtoParser parser(contents.Parser());

        switch (methodId)
        {
            uint32_t value;

            case idMethod:
            {
                while (!parser.Empty())
                {
                    infra::ProtoParser::Field field = parser.GetField();

                    switch (field.second)
                    {
                        case 1:
                            DeserializeField(services::ProtoUInt32(), parser, field, value);
                            break;
                        default:
                            if (field.first.Is<infra::ProtoLengthDelimited>())
                                field.first.Get<infra::ProtoLengthDelimited>().SkipEverything();
                            break;
                    }
                }

                if (!parser.FormatFailed())
                    Method(value);
                break;
            }
            default:
                errorPolicy.MethodNotFound(ServiceId(), methodId);
                contents.SkipEverything();
        }
    }

    TestService1Proxy::TestService1Proxy(services::Echo& echo)
        : services::ServiceProxy(echo, maxMessageSize)
    {}

    void TestService1Proxy::Method(uint32_t value)
    {
        infra::DataOutputStream::WithErrorPolicy stream(Rpc().SendStreamWriter());
        infra::ProtoFormatter formatter(stream);
        formatter.PutVarInt(serviceId);
        {
            infra::ProtoLengthDelimitedFormatter argumentFormatter = formatter.LengthDelimitedFormatter(idMethod);
            SerializeField(services::ProtoUInt32(), formatter, value, 1);
        }
        Rpc().Send();
    }

    class MessageCommunicationMock
        : public services::MessageCommunication
    {
    public:
        MOCK_METHOD1(RequestSendMessage, void(uint16_t size));
        MOCK_CONST_METHOD0(MaxSendMessageSize, std::size_t());
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

    testing::StrictMock<EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<MessageCommunicationMock> messageCommunication;
    services::EchoOnMessageCommunication echo{ messageCommunication, errorPolicy };

    TestService1Proxy serviceProxy{ echo };
    testing::StrictMock<TestService1Mock> service{ echo };
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
    EXPECT_CALL(errorPolicy, ServiceNotFound(2));
    ReceiveMessage(infra::ConstructBin()({ 2, 10, 2, 8, 5 }).Range());
}

TEST_F(EchoOnMessageCommunicationTest, MethodNotFound_is_reported)
{
    EXPECT_CALL(errorPolicy, MethodNotFound(1, 2));
    ReceiveMessage(infra::ConstructBin()({ 1, 18, 2, 8, 5 }).Range());
}
