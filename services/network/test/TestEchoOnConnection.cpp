#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
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
    testing::StrictMock<services::ConnectionMock> connection;
    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> serializerFactory;
    services::EchoOnConnection echo{ serializerFactory, errorPolicy };
    testing::StrictMock<services::ServiceStub> service{ echo };
};

TEST_F(EchoOnConnectionTest, invoke_service_proxy_method)
{
    services::ServiceStubProxy serviceProxy{ echo };

    EXPECT_CALL(connection, MaxSendStreamSize()).WillOnce(testing::Return(1000));
    EXPECT_CALL(connection, RequestSendStream(38));
    serviceProxy.RequestSend([&serviceProxy]()
        {
            serviceProxy.Method(5);
        });

    infra::ByteOutputStreamWriter::WithStorage<128> writer;
    auto writerPtr = infra::UnOwnedSharedPtr(writer);
    connection.Observer().SendStreamAvailable(writerPtr);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 10, 2, 8, 5 }), (std::vector<uint8_t>(writer.Storage().begin(), writer.Storage().begin() + 5)));
}

TEST_F(EchoOnConnectionTest, service_method_is_invoked)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 10, 2, 8, 5 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, service_method_is_invoked_twice)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 10, 2, 8, 5, 1, 10, 2, 8, 6 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(service, Method(5));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();

    EXPECT_CALL(service, Method(6)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    EXPECT_CALL(connection, AckReceived());
    service.MethodDone();
}

TEST_F(EchoOnConnectionTest, MessageFormatError_is_reported_when_message_is_not_a_LengthDelimited)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 0, 2 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(errorPolicy, MessageFormatError());
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, MessageFormatError_is_reported_when_message_is_of_unknown_type)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 6 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(errorPolicy, MessageFormatError());
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, MessageFormatError_is_reported_when_parameter_in_message_is_of_incorrect_type)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 10, 5, 13, 5, 0, 0, 0 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(errorPolicy, MessageFormatError());
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, ServiceNotFound_is_reported)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 2, 10, 2, 8, 5 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(errorPolicy, ServiceNotFound(2));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, MethodNotFound_is_reported)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 18, 2, 8, 5 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(errorPolicy, MethodNotFound(1, 2));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();
}

TEST_F(EchoOnConnectionTest, DataReceived_while_service_method_is_executing)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ 1, 10, 2, 8, 5 });
    auto readerPtr = infra::UnOwnedSharedPtr(stream.Reader());
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(service, Method(5));
    EXPECT_CALL(connection, AckReceived());
    connection.Observer().DataReceived();

    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    connection.Observer().DataReceived();

    service.MethodDone();
}
