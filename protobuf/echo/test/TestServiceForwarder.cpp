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
    services::ServiceForwarderAll forwarder{ echoFrom, echoTo };
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
};

TEST_F(ServiceForwarderAllTest, accept_any_service)
{
    echoFrom.NotifyObservers([](auto& service)
        {
            EXPECT_TRUE(service.AcceptsService(1));
            EXPECT_TRUE(service.AcceptsService(2));
            EXPECT_TRUE(service.AcceptsService(3));
        });
}

TEST_F(ServiceForwarderAllTest, forward_message)
{
    infra::StdVectorOutputStreamWriter::WithStorage writer;

    echoFrom.NotifyObservers([this, &writer](auto& service)
        {
            infra::StdVectorInputStream::WithStorage inputStream{ infra::inPlace, std::vector<uint8_t>{ 1, 2, 3, 4, 5 } };

            EXPECT_CALL(echoTo, RequestSend(testing::_)).WillOnce(testing::Invoke([this, &service, &writer](services::ServiceProxy& serviceProxy)
                {
                    EXPECT_CALL(echoFrom, ServiceDone());
                    auto serializer = serviceProxy.GrantSend();
                    EXPECT_FALSE(serializer->SendStreamAvailable(infra::UnOwnedSharedPtr(writer)));

                }));
            auto deserializer = service.StartMethod(1, 5, 5, errorPolicy);
            deserializer->MethodContents(infra::UnOwnedSharedPtr(inputStream.Reader()));
            deserializer->ExecuteMethod();
        });

    EXPECT_EQ((std::vector<uint8_t>{ 1, 42, 5, 1, 2, 3, 4, 5 }), writer.Storage());
}

class ServiceForwarderTest
    : public testing::Test
{
public:
    testing::StrictMock<services::EchoMock> echoFrom;
    testing::StrictMock<services::EchoMock> echoTo;
    services::ServiceForwarder forwarder{ echoFrom, 1, echoTo };
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
};

TEST_F(ServiceForwarderTest, accept_one_service)
{
    echoFrom.NotifyObservers([](auto& service)
        {
            EXPECT_TRUE(service.AcceptsService(1));
            EXPECT_FALSE(service.AcceptsService(2));
            EXPECT_FALSE(service.AcceptsService(3));
        });
}
