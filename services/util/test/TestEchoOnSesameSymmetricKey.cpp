#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoOnSesameSymmetricKey.hpp"
#include "services/util/test_doubles/SesameMock.hpp"

class EchoOnSesameSymmetricKeyTest
    : public testing::Test
{
public:
    EchoOnSesameSymmetricKeyTest()
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

    void ExpectGenerationOfKeyMaterial(const services::SesameSecured::KeyType& key, const services::SesameSecured::IvType& iv)
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

    services::MethodSerializerFactory::ForServices<services::ServiceStub, sesame_security::SymmetricKeyEstablishment>::AndProxies<services::ServiceStubProxy, sesame_security::SymmetricKeyEstablishmentProxy> serializerFactory;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<hal::SynchronousRandomDataGeneratorMock> randomDataGenerator;
    testing::StrictMock<services::SesameMock> lower;
    std::array<uint8_t, services::SesameSecured::keySize> key{ 1, 2 };
    std::array<uint8_t, services::SesameSecured::blockSize> iv{ 1, 3 };
    services::SesameSecured::WithCryptoMbedTls::WithBuffers<64> secured{ lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    services::EchoOnSesameSymmetricKey echo{ secured, randomDataGenerator, serializerFactory, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echo };
    testing::StrictMock<services::ServiceStub> service{ echo };
};

TEST_F(EchoOnSesameSymmetricKeyTest, send_and_receive)
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

TEST_F(EchoOnSesameSymmetricKeyTest, send_and_receive_large_message)
{
    static const std::array<uint8_t, 64> bytes{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64 };
    EXPECT_CALL(lower, RequestSendMessage(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(lower, RequestSendMessage(testing::_));
            LoopBackData();

            EXPECT_CALL(service, MethodBytes(testing::_)).WillOnce(testing::Invoke([this](const infra::BoundedVector<uint8_t>& value)
                {
                    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(bytes), infra::MakeRange(value)));
                    service.MethodDone();
                }));
            LoopBackData();
        }));

    serviceProxy.RequestSend([this]()
        {
            serviceProxy.MethodBytes(bytes);
        });
}

TEST_F(EchoOnSesameSymmetricKeyTest, send_while_initializing)
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
