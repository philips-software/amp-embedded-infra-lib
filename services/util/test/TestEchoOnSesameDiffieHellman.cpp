#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/util/EchoOnSesameDiffieHellman.hpp"
#include "services/util/test_doubles/SesameMock.hpp"

class EchoOnSesameDiffieHellmanTest
    : public testing::Test
{
public:
    EchoOnSesameDiffieHellmanTest()
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

        Initialized();
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

    services::MethodSerializerFactory::ForServices<services::ServiceStub, sesame_security::DiffieHellmanKeyEstablishment>::AndProxies<services::ServiceStubProxy, sesame_security::DiffieHellmanKeyEstablishmentProxy> serializerFactory;
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
    services::SesameSecured::WithBuffers<1024> securedLeft{ lowerLeft, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    services::SesameSecured::WithBuffers<1024> securedRight{ lowerRight, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    std::string rootCaCertificate{ R"(-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEIu5HmQLohOLllaKhB6xULooVWzjR
kS6C2YYGPHDd4GkIUux9bCz2OCiTaiw6v2NF2rES9XP2BdHh/g2aZMyxPQ==
-----END PUBLIC KEY-----
)" };
    std::string dsaCertificateLeft{ R"(-----BEGIN CERTIFICATE-----
MIIB4DCCAYWgAwIBAgIUBGDybnU/VS6hMykUiS8N6Y68itEwCgYIKoZIzj0EAwIw
RTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGElu
dGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yNTAzMjAxMjQ2MTZaFw0yNTA0MTkx
MjQ2MTZaMEUxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYD
VQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwWTATBgcqhkjOPQIBBggqhkjO
PQMBBwNCAASawvuaBpyB3gtfQHtTcH9hoRcjNtR2p2NhnJiEAAPz3DUhxO9fAY97
s4i134wrpsxAhgiMsaMge49Ls2AJykBNo1MwUTAdBgNVHQ4EFgQUFJfjqWn1cdjz
TAw8GlaHCPmcBUowHwYDVR0jBBgwFoAUFJfjqWn1cdjzTAw8GlaHCPmcBUowDwYD
VR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNJADBGAiEA3lyvoLu3a2firw25vA8X
v3xdQXsVBemc793h7qscbsICIQDYgfo9xTqNNLfQJFPFGYk/eXR8ut2MU7SE0vJv
F1QTTQ==
-----END CERTIFICATE-----

)" };
    std::string dsaCertificatePrivateKeyLeft{ R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgwMDRnRGBKL1+2owr
CZkxpxutvGb/+/HiyIsxEfOnslyhRANCAASawvuaBpyB3gtfQHtTcH9hoRcjNtR2
p2NhnJiEAAPz3DUhxO9fAY97s4i134wrpsxAhgiMsaMge49Ls2AJykBN
-----END PRIVATE KEY-----
)" };
    std::string dsaCertificateRight{ R"(-----BEGIN CERTIFICATE-----
MIIB3zCCAYWgAwIBAgIUafGvdT6+RG2nTLwAA+4wVqyrJWcwCgYIKoZIzj0EAwIw
RTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGElu
dGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yNTAzMjAxMjQxNTdaFw0yNTA0MTkx
MjQxNTdaMEUxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYD
VQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwWTATBgcqhkjOPQIBBggqhkjO
PQMBBwNCAASvE6STQqYizMi8miss+xv4KijNeAWrVUizpANknfuMr+9QzxCdtK91
H+M9oIkHc8uBSHbp0SyKPjyNZlcnaKaio1MwUTAdBgNVHQ4EFgQUpKSiLD698j2w
RDSGfcaObsnMzO4wHwYDVR0jBBgwFoAUpKSiLD698j2wRDSGfcaObsnMzO4wDwYD
VR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNIADBFAiB+UL6K69e9zoNUWcvYfX3p
wZYDQZuYiLlRcusnKAcCsAIhAPLzl+4fu36XWIg4pdClNWmOzkkZrrNQjd1vgX1L
Yhj8
-----END CERTIFICATE-----

)" };
    std::string dsaCertificatePrivateKeyRight{ R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgzOV9zLb8W5IdilFO
mCjb6HTfdpG3mp6jd0x255PIt8uhRANCAASvE6STQqYizMi8miss+xv4KijNeAWr
VUizpANknfuMr+9QzxCdtK91H+M9oIkHc8uBSHbp0SyKPjyNZlcnaKai
-----END PRIVATE KEY-----
)" };
    infra::Execute e2{
        [this]()
        {
            rootCaCertificate.push_back(0);
            dsaCertificateLeft.push_back(0);
            dsaCertificatePrivateKeyLeft.push_back(0);
            dsaCertificateRight.push_back(0);
            dsaCertificatePrivateKeyRight.push_back(0);
        }
    };
    services::EchoOnSesameDiffieHellman echoLeft{ securedLeft, dsaCertificateLeft, dsaCertificatePrivateKeyLeft, rootCaCertificate, randomDataGenerator, serializerFactory, errorPolicy };
    services::EchoOnSesameDiffieHellman echoRight{ securedRight, dsaCertificateRight, dsaCertificatePrivateKeyRight, rootCaCertificate, randomDataGenerator, serializerFactory, errorPolicy };

    services::ServiceStubProxy serviceProxy{ echoLeft };
    testing::StrictMock<services::ServiceStub> service{ echoRight };
};

TEST_F(EchoOnSesameDiffieHellmanTest, send_and_receive)
{
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
