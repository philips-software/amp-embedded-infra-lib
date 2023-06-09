#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/EchoOnConnection.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"

class EchoOnConnectionTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    EchoOnConnectionTest()
    {
        connection.Attach(infra::UnOwnedSharedPtr(echo));
    }

    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    services::ConnectionMock connection;
    services::EchoOnConnection echo{ errorPolicy };
    testing::StrictMock<services::ServiceStub> service{ echo };
};

TEST_F(EchoOnConnectionTest, invoke_service_proxy_method)
{
    services::ServiceStubProxy serviceProxy{ echo };

    testing::StrictMock<infra::MockCallback<void()>> onGranted;
    EXPECT_CALL(connection, RequestSendStream(18));
    serviceProxy.RequestSend([&onGranted]()
        {
            onGranted.callback();
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    auto writerPtr = infra::UnOwnedSharedPtr(writer);
    EXPECT_CALL(onGranted, callback());
    connection.Observer().SendStreamAvailable(writerPtr);

    serviceProxy.Method(5);
    EXPECT_EQ((std::vector<uint8_t>{ 1, 10, 2, 8, 5 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 5)));
}

TEST_F(EchoOnConnectionTest, service_method_is_invoked)
{
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
