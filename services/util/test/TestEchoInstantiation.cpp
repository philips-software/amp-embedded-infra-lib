#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/util/EchoInstantiation.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "gmock/gmock.h"

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class EchoInstantiation
    {
    public:
        services::SerialCommunicationLoopback serial;

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<LeftSize> leftSerial{ serial.Server() };
        services::MethodSerializerFactory::OnHeap leftSerializerFactory;
        main_::EchoOnSesame::WithMessageSize<LeftSize, 2> leftEcho{ leftSerial, leftSerializerFactory };

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<RightSize> rightSerial{ serial.Client() };
        services::MethodSerializerFactory::OnHeap rightSerializerFactory;
        main_::EchoOnSesame::WithMessageSize<RightSize, 2> rightEcho{ rightSerial, rightSerializerFactory };

        services::ServiceStubProxy serviceProxy{ leftEcho.echo };
        testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };
    };
}

class EchoInstantiationTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
    , public EchoInstantiation<256, 1024>
{};

TEST_F(EchoInstantiationTest, send_message)
{
    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));

    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    ExecuteAllActions();
}

TEST_F(EchoInstantiationTest, send_multiple_messages)
{
    EXPECT_CALL(service, Method(5)).Times(3).WillRepeatedly([this]()
        {
            service.MethodDone();
        });

    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);

            serviceProxy.RequestSend([this]()
                {
                    serviceProxy.Method(5);

                    serviceProxy.RequestSend([this]()
                        {
                            serviceProxy.Method(5);
                        });
                });
        });

    ExecuteAllActions();
}
