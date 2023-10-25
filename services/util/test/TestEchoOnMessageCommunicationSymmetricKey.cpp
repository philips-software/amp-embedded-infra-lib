#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoOnMessageCommunicationSymmetricKey.hpp"
#include "services/util/test_doubles/MessageCommunicationMock.hpp"

class EchoOnMessageCommunicationSymmetricKeyTest
    : public testing::Test
{
public:
    EchoOnMessageCommunicationSymmetricKeyTest()
    {
        EXPECT_CALL(lower, MaxSendMessageSize()).WillRepeatedly(testing::Return(100));

        Initialized();
    }

    void Initialized()
    {
        EXPECT_CALL(lower, RequestSendMessage(testing::_)).WillOnce(testing::Invoke([this]()
            {
                ExpectGenerationOfKeyMaterial({ 4 }, { 5 });
                LoopBackData();
            }));
        lower.GetObserver().Initialized();
    }

    void ExpectGenerationOfKeyMaterial(const services::MessageCommunicationSecured::KeyType& key, const services::MessageCommunicationSecured::IvType& iv)
    {
        // clang-format off
        EXPECT_CALL(randomDataGenerator, GenerateRandomData(testing::_)).WillOnce(testing::Invoke([this, key](infra::ByteRange range)
            {
                infra::Copy(key, range);
            })).WillOnce(testing::Invoke([this, iv](infra::ByteRange range)
            {
                infra::Copy(iv, range);
            }));
        // clang-format on
    }

    void LoopBackData()
    {
        std::vector<uint8_t> sentData;

        infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
        lower.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));
        ASSERT_TRUE(writer.Allocatable());

        infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
        lower.GetObserver().ReceivedMessage(reader.Emplace(sentData));
        ASSERT_TRUE(reader.Allocatable());
    }

    services::MethodSerializerFactory::ForServices<services::ServiceStub, message_communication_security::SymmetricKeyEstablishment>::AndProxies<services::ServiceStubProxy, message_communication_security::SymmetricKeyEstablishmentProxy> serializerFactory;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<hal::SynchronousRandomDataGeneratorMock> randomDataGenerator;
    testing::StrictMock<services::MessageCommunicationMock> lower;
    std::array<uint8_t, services::MessageCommunicationSecured::keySize> key{ 1, 2 };
    std::array<uint8_t, services::MessageCommunicationSecured::blockSize> iv{ 1, 3 };
    services::MessageCommunicationSecured::WithBuffers<64> secured{ lower, key, iv, key, iv };
    services::EchoOnMessageCommunicationSymmetricKey echo{ secured, serializerFactory, randomDataGenerator, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echo, serializerFactory };
    testing::StrictMock<services::ServiceStub> service{ echo, serializerFactory };
};

TEST_F(EchoOnMessageCommunicationSymmetricKeyTest, send_and_receive)
{
    EXPECT_CALL(lower, RequestSendMessage(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
                {
                    service.MethodDone();
                }));
            LoopBackData();
        }));

    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });
}

TEST_F(EchoOnMessageCommunicationSymmetricKeyTest, send_while_initializing)
{
    EXPECT_CALL(lower, RequestSendMessage(testing::_));
    lower.GetObserver().Initialized();

    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });

    EXPECT_CALL(lower, RequestSendMessage(testing::_));

    ExpectGenerationOfKeyMaterial({ 4 }, { 5 });
    LoopBackData();

    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));
    LoopBackData();
}
