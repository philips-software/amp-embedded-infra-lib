#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
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

        constexpr static std::size_t leftSize = LeftSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<leftSize> leftSerial{ serial.Server() };
        services::MethodSerializerFactory::OnHeap leftSerializerFactory;
        main_::EchoOnSesame::WithMessageSize<leftSize, 2> leftEcho{ leftSerial, leftSerializerFactory };

        constexpr static std::size_t rightSize = RightSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<rightSize> rightSerial{ serial.Client() };
        services::MethodSerializerFactory::OnHeap rightSerializerFactory;
        main_::EchoOnSesame::WithMessageSize<rightSize, 2> rightEcho{ rightSerial, rightSerializerFactory };

        services::ServiceStubProxy serviceProxy{ leftEcho.echo };
        testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };
    };
}

class EchoInstantiationTest
    : public testing::Test
    , public infra::ClockFixture
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
    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
                {
                    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
                        {
                            service.MethodDone();
                        }));
                    service.MethodDone();
                }));
            service.MethodDone();
        }));

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
