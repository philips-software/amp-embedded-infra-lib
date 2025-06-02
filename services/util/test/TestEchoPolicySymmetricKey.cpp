#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoPolicySymmetricKey.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include <numeric>

class EchoPolicySymmetricKeyTest
    : public testing::Test
{
public:
    EchoPolicySymmetricKeyTest()
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
            })).RetiresOnSaturation();
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
    infra::Execute e{ [this]()
        {
            ExpectGenerationOfKeyMaterial({ 1, 2 }, { 3, 4 });
            ExpectGenerationOfKeyMaterial({ 1, 2 }, { 3, 4 });
        } };
    sesame_security::SymmetricKeyFile keys{ services::GenerateSymmetricKeys(randomDataGenerator) };
    services::SesameSecured::WithCryptoMbedTls::WithBuffers<64> secured{ lower, keys };
    services::EchoOnSesame echo{ secured, serializerFactory, errorPolicy };
    services::EchoPolicySymmetricKey policy{ echo, echo, secured, randomDataGenerator };

    services::ServiceStubProxy serviceProxy{ echo };
    testing::StrictMock<services::ServiceStub> service{ echo };
};

TEST_F(EchoPolicySymmetricKeyTest, send_and_receive)
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

TEST_F(EchoPolicySymmetricKeyTest, send_and_receive_large_message)
{
    std::array<uint8_t, 64> bytes;
    std::iota(bytes.begin(), bytes.end(), 1);
    EXPECT_CALL(lower, RequestSendMessage(testing::_)).WillOnce(testing::Invoke([this, &bytes]()
        {
            EXPECT_CALL(lower, RequestSendMessage(testing::_));
            LoopBackData();

            EXPECT_CALL(service, MethodBytes(testing::_)).WillOnce(testing::Invoke([this, &bytes](const infra::BoundedVector<uint8_t>& value)
                {
                    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(bytes), infra::MakeRange(value)));
                    service.MethodDone();
                }));
            LoopBackData();
        }));

    serviceProxy.RequestSend([this, &bytes]()
        {
            serviceProxy.MethodBytes(bytes);
        });
}

TEST_F(EchoPolicySymmetricKeyTest, send_while_initializing)
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
