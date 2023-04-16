#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "protobuf/echo/ServiceForwarder.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"

class ServiceForwarderAllTest
    : public testing::Test
{
public:
    testing::StrictMock<services::EchoMock> echoFrom;
    testing::StrictMock<services::EchoMock> echoTo;
    services::ServiceForwarderAll::WithMaxMessageSize<100> forwarder{ echoFrom, echoTo };
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
};

TEST_F(ServiceForwarderAllTest, accept_any_service)
{
    echoFrom.NotifyObservers([](auto& service)
        {
        EXPECT_TRUE(service.AcceptsService(1));
        EXPECT_TRUE(service.AcceptsService(2));
        EXPECT_TRUE(service.AcceptsService(3)); });
}

TEST_F(ServiceForwarderAllTest, forward_message)
{
    echoFrom.NotifyObservers([this](auto& service)
        {
            infra::StreamErrorPolicy streamErrorPolicy;
            infra::StdVectorInputStream::WithStorage inputStream{ infra::inPlace, std::vector<uint8_t>{ 1, 2, 3, 4, 5 } };
            infra::ProtoLengthDelimited contents{ inputStream, streamErrorPolicy, static_cast<uint32_t>(inputStream.Available()) };

            EXPECT_CALL(echoTo, RequestSend(testing::_)).WillOnce(testing::Invoke([this, &service](services::ServiceProxy& serviceProxy)
                {
                    infra::StdVectorOutputStreamWriter::WithStorage writer;
                    EXPECT_CALL(echoTo, SendStreamWriter()).WillOnce(testing::ReturnRef(writer));
                    EXPECT_CALL(echoTo, Send());
                    EXPECT_CALL(echoFrom, ServiceDone(testing::Ref(service)));
                    serviceProxy.GrantSend();

                    EXPECT_EQ((std::vector<uint8_t>{ 1, 42, 5, 1, 2, 3, 4, 5 }), writer.Storage());
                }));
            service.HandleMethod(1, 5, contents, errorPolicy);
        });
}

class ServiceForwarderTest
    : public testing::Test
{
public:
    testing::StrictMock<services::EchoMock> echoFrom;
    testing::StrictMock<services::EchoMock> echoTo;
    services::ServiceForwarder::WithMaxMessageSize<100> forwarder{ echoFrom, 1, echoTo };
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
};

TEST_F(ServiceForwarderTest, accept_one_service)
{
    echoFrom.NotifyObservers([](auto& service)
        {
        EXPECT_TRUE(service.AcceptsService(1));
        EXPECT_FALSE(service.AcceptsService(2));
        EXPECT_FALSE(service.AcceptsService(3)); });
}
