#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "protobuf/echo/EchoOnStreams.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "gmock/gmock.h"

namespace services
{
    class EchoOnStreamsMock
        : public EchoOnStreams
    {
    public:
        using EchoOnStreams::EchoOnStreams;

        MOCK_METHOD(infra::SharedPtr<MethodSerializer>, GrantSend, (ServiceProxy & proxy), (override));
        MOCK_METHOD(infra::SharedPtr<MethodDeserializer>, StartingMethod, (uint32_t serviceId, uint32_t methodId, infra::SharedPtr<MethodDeserializer>&& deserializer), (override));
        MOCK_METHOD(void, RequestSendStream, (std::size_t size), (override));
        MOCK_METHOD(void, MethodContents, (infra::SharedPtr<infra::StreamReaderWithRewinding> && reader), (override));
        MOCK_METHOD(void, ReleaseDeserializer, (), (override));

        using EchoOnStreams::DataReceived;
        using EchoOnStreams::Initialized;
        using EchoOnStreams::ReleaseReader;
        using EchoOnStreams::SendStreamAvailable;

        infra::SharedPtr<MethodSerializer> InheritedGrantSend(ServiceProxy& proxy)
        {
            return EchoOnStreams::GrantSend(proxy);
        }
    };
}

class EchoOnStreamsTest
    : public testing::Test
{
public:
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> serializerFactory;
    testing::StrictMock<services::EchoOnStreamsMock> echo{ serializerFactory, errorPolicy };
    testing::StrictMock<services::ServiceStub> service{ echo };
    services::ServiceStubProxy serviceProxy{ echo };

    std::vector<uint8_t> data;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
};

TEST_F(EchoOnStreamsTest, send_method_without_parameter)
{
    EXPECT_CALL(echo, RequestSendStream(services::ServiceStubProxy::maxMessageSize + 2 * 10));
    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodNoParameter();
        });

    EXPECT_CALL(echo, GrantSend(testing::Ref(serviceProxy))).WillOnce(testing::Invoke([this](services::ServiceProxy& serviceProxy)
        {
            return echo.InheritedGrantSend(serviceProxy);
        }));
    echo.SendStreamAvailable(writer.Emplace(data));
    EXPECT_EQ((std::vector<uint8_t>{1, 26, 0}), data);
}
