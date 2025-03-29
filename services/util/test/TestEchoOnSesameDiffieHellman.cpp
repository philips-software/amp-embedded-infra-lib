#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoOnSesameDiffieHellman.hpp"
#include "services/util/MbedTlsRandomDataGeneratorWrapper.hpp"
#include "services/util/test_doubles/SesameMock.hpp"

namespace
{
    class EchoOnSesameDiffieHellmanWithCryptoMbedTlsMock
        : public services::EchoOnSesameDiffieHellman::WithCryptoMbedTls
    {
    public:
        using services::EchoOnSesameDiffieHellman::WithCryptoMbedTls::WithCryptoMbedTls;

        MOCK_METHOD(void, KeyExchangeSuccessful, (), (override));
        MOCK_METHOD(void, KeyExchangeFailed, (), (override));
    };
}

class EchoOnSesameDiffieHellmanTestBase
    : public testing::Test
{
public:
    EchoOnSesameDiffieHellmanTestBase()
    {
        EXPECT_CALL(lowerLeft, MaxSendMessageSize()).WillRepeatedly(testing::Return(10000));
        EXPECT_CALL(lowerRight, MaxSendMessageSize()).WillRepeatedly(testing::Return(10000));

        EXPECT_CALL(lowerLeft, RequestSendMessage(testing::_)).WillRepeatedly(testing::Invoke([this]()
            {
                assert(!lowerLeftRequest);
                lowerLeftRequest = true;
            }));

        EXPECT_CALL(lowerRight, RequestSendMessage(testing::_)).WillRepeatedly(testing::Invoke([this]()
            {
                assert(!lowerRightRequest);
                lowerRightRequest = true;
            }));
    }

    void Initialized()
    {
        lowerLeft.GetObserver().Initialized();
        lowerRight.GetObserver().Initialized();
    }

    void ExpectGenerationOfKeyMaterial()
    {
        EXPECT_CALL(randomDataGenerator, GenerateRandomData(testing::_)).WillRepeatedly(testing::Invoke([](infra::ByteRange data)
            {
                static uint8_t fill = 0;
                std::fill(data.begin(), data.end(), fill++);
            }));
    }

    void ExchangeData()
    {
        while (lowerLeftRequest || lowerRightRequest)
        {
            if (lowerLeftRequest)
                ExchangeMessage(lowerLeftRequest, lowerLeft, lowerRight);

            if (lowerRightRequest)
                ExchangeMessage(lowerRightRequest, lowerRight, lowerLeft);
        }
    }

    void ExchangeMessage(bool& request, services::Sesame& x, services::Sesame& y)
    {
        request = false;
        std::vector<uint8_t> sentData;

        infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
        x.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));
        ASSERT_TRUE(writer.Allocatable());

        infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
        y.GetObserver().ReceivedMessage(reader.Emplace(sentData));
        ASSERT_TRUE(reader.Allocatable());
    }

    services::MethodSerializerFactory::ForServices<services::ServiceStub, sesame_security::DiffieHellmanKeyEstablishment>::AndProxies<services::ServiceStubProxy, sesame_security::DiffieHellmanKeyEstablishmentProxy> serializerFactoryLeft;
    services::MethodSerializerFactory::ForServices<services::ServiceStub, sesame_security::DiffieHellmanKeyEstablishment>::AndProxies<services::ServiceStubProxy, sesame_security::DiffieHellmanKeyEstablishmentProxy> serializerFactoryRight;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<hal::SynchronousRandomDataGeneratorMock> randomDataGenerator;
    infra::Execute e{
        [this]()
        {
            ExpectGenerationOfKeyMaterial();
        }
    };
    testing::StrictMock<services::SesameMock> lowerLeft;
    testing::StrictMock<services::SesameMock> lowerRight;
    bool lowerLeftRequest = false;
    bool lowerRightRequest = false;
    std::array<uint8_t, services::SesameSecured::keySize> key{ 1, 2 };
    std::array<uint8_t, services::SesameSecured::blockSize> iv{ 1, 3 };
    services::SesameSecured::WithCryptoMbedTls::WithBuffers<100> securedLeft{ lowerLeft, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    services::SesameSecured::WithCryptoMbedTls::WithBuffers<100> securedRight{ lowerRight, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };

    services::EcSecP256r1PrivateKey rootCaPrivateKey{ randomDataGenerator };
    services::EcSecP256r1Certificate rootCaCertificate{ rootCaPrivateKey, "CN=Root", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    std::string rootCaCertificatePem{ infra::AsStdString(rootCaCertificate.Pem()) };

    services::EcSecP256r1PrivateKey privateKeyLeft{ randomDataGenerator };
    std::string privateKeyLeftPem{ infra::AsStdString(privateKeyLeft.Pem()) };
    services::EcSecP256r1Certificate certificateLeft{ privateKeyLeft, "CN=left", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    std::string certificateLeftPem{ infra::AsStdString(certificateLeft.Pem()) };

    services::EcSecP256r1PrivateKey privateKeyRight{ randomDataGenerator };
    std::string privateKeyRightPem{ infra::AsStdString(privateKeyRight.Pem()) };
    services::EcSecP256r1Certificate certificateRight{ privateKeyRight, "CN=right", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    std::string certificateRightPem{ infra::AsStdString(certificateRight.Pem()) };

    testing::StrictMock<EchoOnSesameDiffieHellmanWithCryptoMbedTlsMock> echoLeft{ securedLeft, certificateLeftPem, privateKeyLeftPem, rootCaCertificatePem, randomDataGenerator, serializerFactoryLeft, errorPolicy };
};

class EchoOnSesameDiffieHellmanTest
    : public EchoOnSesameDiffieHellmanTestBase
{
public:
    EchoOnSesameDiffieHellmanTest()
    {
        Initialized();
    }

    testing::StrictMock<EchoOnSesameDiffieHellmanWithCryptoMbedTlsMock> echoRight{ securedRight, certificateRightPem, privateKeyRightPem, rootCaCertificatePem, randomDataGenerator, serializerFactoryRight, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echoLeft };
    testing::StrictMock<services::ServiceStub> service{ echoRight };
};

TEST_F(EchoOnSesameDiffieHellmanTest, send_and_receive)
{
    EXPECT_CALL(echoLeft, KeyExchangeSuccessful());
    EXPECT_CALL(echoRight, KeyExchangeSuccessful());

    EXPECT_CALL(service, Method(5)).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));

    serviceProxy.RequestSend([this]()
        {
            serviceProxy.Method(5);
        });
    ExchangeData();
}

namespace
{
    class DiffieHellmanKeyEstablishmentMock
        : public sesame_security::DiffieHellmanKeyEstablishment
    {
    public:
        using sesame_security::DiffieHellmanKeyEstablishment::DiffieHellmanKeyEstablishment;

        MOCK_METHOD(void, Exchange, (infra::ConstByteRange publicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS), (override));
        MOCK_METHOD(void, PresentCertificate, (infra::BoundedConstString certificate), (override));
    };
}

class EchoOnSesameDiffieHellmanAdversaryTest
    : public EchoOnSesameDiffieHellmanTestBase
{
public:
    services::EchoOnSesame echoRight{ securedRight, serializerFactoryRight };
    testing::StrictMock<DiffieHellmanKeyEstablishmentMock> keyEstablishment{ echoRight };
    sesame_security::DiffieHellmanKeyEstablishmentProxy proxy{ echoRight };

    services::EcSecP256r1DiffieHellmanMbedTls keyExchange{ randomDataGenerator };
    services::EcSecP256r1DsaSignerMbedTls signer{ privateKeyRightPem, randomDataGenerator };
};

TEST_F(EchoOnSesameDiffieHellmanAdversaryTest, successful_manual_implementation)
{
    Initialized();

    proxy.RequestSend([this]()
        {
            proxy.PresentCertificate(certificateRightPem);
        });

    EXPECT_CALL(keyEstablishment, PresentCertificate(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(keyEstablishment, Exchange(testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([this](infra::ConstByteRange publicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
                {
                    proxy.RequestSend([this]()
                        {
                            auto encodedDhPublicKey = keyExchange.PublicKey();
                            auto [r, s] = signer.Sign(encodedDhPublicKey);

                            proxy.Exchange(encodedDhPublicKey, r, s);
                        });

                    keyEstablishment.MethodDone();
                }));

            keyEstablishment.MethodDone();
        }));

    EXPECT_CALL(echoLeft, KeyExchangeSuccessful());

    ExchangeData();
}

TEST_F(EchoOnSesameDiffieHellmanAdversaryTest, self_signed_certificate_leads_to_failure)
{
    Initialized();

    services::EcSecP256r1Certificate certificateRightSelfSigned{ privateKeyRight, "CN=right", privateKeyRight, "CN=Root", randomDataGenerator };
    std::string certificateRightSelfSignedPem{ infra::AsStdString(certificateRightSelfSigned.Pem()) };

    proxy.RequestSend([this, &certificateRightSelfSignedPem]()
        {
            proxy.PresentCertificate(certificateRightSelfSignedPem);
        });

    EXPECT_CALL(keyEstablishment, PresentCertificate(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(keyEstablishment, Exchange(testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([this](infra::ConstByteRange publicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
                {
                    proxy.RequestSend([this]()
                        {
                            auto encodedDhPublicKey = keyExchange.PublicKey();
                            auto [r, s] = signer.Sign(encodedDhPublicKey);

                            proxy.Exchange(encodedDhPublicKey, r, s);
                        });

                    keyEstablishment.MethodDone();
                }));

            keyEstablishment.MethodDone();
        }));

    EXPECT_CALL(echoLeft, KeyExchangeFailed());

    ExchangeData();
}

TEST_F(EchoOnSesameDiffieHellmanAdversaryTest, signature_over_incorrect_data_leads_to_failure)
{
    Initialized();

    proxy.RequestSend([this]()
        {
            proxy.PresentCertificate(certificateRightPem);
        });

    EXPECT_CALL(keyEstablishment, PresentCertificate(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(keyEstablishment, Exchange(testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([this](infra::ConstByteRange publicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
                {
                    proxy.RequestSend([this]()
                        {
                            auto encodedDhPublicKey = keyExchange.PublicKey();
                            auto encodedDhPublicKeyCopy = encodedDhPublicKey;
                            ++encodedDhPublicKeyCopy[5];
                            auto [r, s] = signer.Sign(encodedDhPublicKeyCopy);

                            proxy.Exchange(encodedDhPublicKey, r, s);
                        });

                    keyEstablishment.MethodDone();
                }));

            keyEstablishment.MethodDone();
        }));

    EXPECT_CALL(echoLeft, KeyExchangeFailed());

    ExchangeData();
}

TEST_F(EchoOnSesameDiffieHellmanAdversaryTest, incorrect_signature_leads_to_failure)
{
    Initialized();

    proxy.RequestSend([this]()
        {
            proxy.PresentCertificate(certificateRightPem);
        });

    EXPECT_CALL(keyEstablishment, PresentCertificate(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(keyEstablishment, Exchange(testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([this](infra::ConstByteRange publicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
                {
                    proxy.RequestSend([this]()
                        {
                            auto encodedDhPublicKey = keyExchange.PublicKey();
                            auto [r, s] = signer.Sign(encodedDhPublicKey);
                            ++r[0];

                            proxy.Exchange(encodedDhPublicKey, r, s);
                        });

                    keyEstablishment.MethodDone();
                }));

            keyEstablishment.MethodDone();
        }));

    EXPECT_CALL(echoLeft, KeyExchangeFailed());

    ExchangeData();
}

TEST_F(EchoOnSesameDiffieHellmanAdversaryTest, not_presenting_certificate_leads_to_failure)
{
    Initialized();

    EXPECT_CALL(keyEstablishment, PresentCertificate(testing::_)).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(keyEstablishment, Exchange(testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([this](infra::ConstByteRange publicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
                {
                    proxy.RequestSend([this]()
                        {
                            auto encodedDhPublicKey = keyExchange.PublicKey();
                            auto [r, s] = signer.Sign(encodedDhPublicKey);

                            proxy.Exchange(encodedDhPublicKey, r, s);
                        });

                    keyEstablishment.MethodDone();
                }));

            keyEstablishment.MethodDone();
        }));

    EXPECT_CALL(echoLeft, KeyExchangeFailed());

    ExchangeData();
}
