#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
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
}

class EchoOnConnectionTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    EchoOnConnectionTest()
    {
        connection.Attach(infra::UnOwnedSharedPtr(echo));
    }

    testing::StrictMock<EchoErrorPolicyMock> errorPolicy;
    services::ConnectionMock connection;
    services::EchoOnConnection echo{ errorPolicy };
};

TEST_F(EchoOnConnectionTest, invoke_service_proxy_method)
{
    TestService1Proxy service{ echo };

    testing::StrictMock<infra::MockCallback<void()>> onGranted;
    EXPECT_CALL(connection, RequestSendStream(18));
    service.RequestSend([&onGranted]()
        { onGranted.callback(); });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    auto writerPtr = infra::UnOwnedSharedPtr(writer);
    EXPECT_CALL(onGranted, callback());
    connection.Observer().SendStreamAvailable(writerPtr);

    service.Method(5);
    EXPECT_EQ((std::vector<uint8_t>{ 1, 10, 2, 8, 5 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 5)));
}

TEST_F(EchoOnConnectionTest, service_method_is_invoked)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 5>{ 1, 10, 2, 8, 5 }), infra::Head(infra::MakeRange(reader.Storage()), 5));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(service, Method(5));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, on_partial_message_service_method_is_not_invoked)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<4> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 4>{ 1, 10, 2, 8 }), infra::Head(infra::MakeRange(reader.Storage()), 4));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, service_method_is_invoked_twice)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 5>{ 1, 10, 2, 8, 5 }), infra::Head(infra::MakeRange(reader.Storage()), 5));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<128> reader2;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 5>{ 1, 10, 2, 8, 5 }), infra::Head(infra::MakeRange(reader2.Storage()), 5));
    auto reader2Ptr = infra::UnOwnedSharedPtr(reader2);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(reader2Ptr));
    EXPECT_CALL(service, Method(5));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();

    reader2.Rewind(0);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(reader2Ptr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(service, Method(5));
    EXPECT_CALL(connection, AckReceived());
    service.MethodDone();
    ExecuteAllActions();
}

TEST_F(EchoOnConnectionTest, MessageFormatError_is_reported_when_message_is_not_a_LengthDelimited)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 3>{ 1, 0, 2 }), infra::Head(infra::MakeRange(reader.Storage()), 3));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(errorPolicy, MessageFormatError());
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, MessageFormatError_is_reported_when_message_is_of_unknown_type)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 2>{ 1, 6 }), infra::Head(infra::MakeRange(reader.Storage()), 2));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(errorPolicy, MessageFormatError());
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, MessageFormatError_is_reported_when_parameter_in_message_is_of_incorrect_type)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 8>{ 1, 10, 2, 13, 5, 0, 0, 0 }), infra::Head(infra::MakeRange(reader.Storage()), 8));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(errorPolicy, MessageFormatError());
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, ServiceNotFound_is_reported)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 5>{ 2, 10, 2, 8, 5 }), infra::Head(infra::MakeRange(reader.Storage()), 5));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(errorPolicy, ServiceNotFound(2));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, MethodNotFound_is_reported)
{
    testing::StrictMock<TestService1Mock> service(echo);

    infra::ByteInputStreamReader::WithStorage<128> reader;
    infra::Copy(infra::MakeRange(std::array<uint8_t, 5>{ 1, 18, 2, 8, 5 }), infra::Head(infra::MakeRange(reader.Storage()), 5));
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    infra::ByteInputStreamReader::WithStorage<0> emptyReader;
    auto emptyReaderPtr = infra::UnOwnedSharedPtr(emptyReader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr)).WillOnce(testing::Return(emptyReaderPtr));
    EXPECT_CALL(errorPolicy, MethodNotFound(1, 2));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}
