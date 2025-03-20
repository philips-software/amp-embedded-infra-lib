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
    std::string rootCaCertificate{ R"(-----BEGIN CERTIFICATE-----
MIICdTCCAhugAwIBAgIULwmcYHDmNNaIjMkxhavqK1YdGvowCgYIKoZIzj0EAwIw
gY8xCzAJBgNVBAYTAk5MMRQwEgYDVQQIDAtOZXRoZXJsYW5kczEMMAoGA1UEBwwD
U29uMRAwDgYDVQQKDAdSaWNoYXJkMRAwDgYDVQQLDAdSaWNoYXJkMRQwEgYDVQQD
DAtycGV0ZXJzLm9yZzEiMCAGCSqGSIb3DQEJARYTcmljaGFyZEBycGV0ZXJzLm9y
ZzAeFw0yNTAzMjAxNjM0MzhaFw0yNzEyMTUxNjM0MzhaMIGPMQswCQYDVQQGEwJO
TDEUMBIGA1UECAwLTmV0aGVybGFuZHMxDDAKBgNVBAcMA1NvbjEQMA4GA1UECgwH
UmljaGFyZDEQMA4GA1UECwwHUmljaGFyZDEUMBIGA1UEAwwLcnBldGVycy5vcmcx
IjAgBgkqhkiG9w0BCQEWE3JpY2hhcmRAcnBldGVycy5vcmcwWTATBgcqhkjOPQIB
BggqhkjOPQMBBwNCAASs2aVQCxjuXi/j+P1nEkBq1vBreaTSIU2qJ7Ef5LAn0KSN
1GrE1YBRSszzs4+E5+0kUNdUiWZlYoCc19H4RwQ8o1MwUTAdBgNVHQ4EFgQUjXoQ
8n3vMX8IHRWwAXEC5i6tUScwHwYDVR0jBBgwFoAUjXoQ8n3vMX8IHRWwAXEC5i6t
UScwDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNIADBFAiEA7KCumPkfTtiN
x8A5IXgZsPlL2YWB4TpNgU5MGd50IBsCIEz0JhuYeidumzxnPdf7iNk7SEyXgBvS
7koJHaaGZIJl
-----END CERTIFICATE-----
)" };
    std::string dsaCertificateLeft{ R"(-----BEGIN CERTIFICATE-----
MIICCjCCAa8CAhI0MAoGCCqGSM49BAMCMIGPMQswCQYDVQQGEwJOTDEUMBIGA1UE
CAwLTmV0aGVybGFuZHMxDDAKBgNVBAcMA1NvbjEQMA4GA1UECgwHUmljaGFyZDEQ
MA4GA1UECwwHUmljaGFyZDEUMBIGA1UEAwwLcnBldGVycy5vcmcxIjAgBgkqhkiG
9w0BCQEWE3JpY2hhcmRAcnBldGVycy5vcmcwHhcNMjUwMzIwMTYzNzA2WhcNMjYw
MzIwMTYzNzA2WjCBjzELMAkGA1UEBhMCTkwxFDASBgNVBAgMC05ldGhlcmxhbmRz
MQwwCgYDVQQHDANzb24xEDAOBgNVBAoMB1JpY2hhcmQxEDAOBgNVBAsMB1JpY2hh
cmQxFDASBgNVBAMMC3JwZXRlcnMub3JnMSIwIAYJKoZIhvcNAQkBFhNyaWNoYXJk
QHJwZXRlcnMub3JnMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEj1db5MMOZ5us
N4bSwnHJnaktm7Lx5OH9Pe0GzJ3R9uO8+ODkGD6C5XUtXLK+Hi3RbmEWLJBR/5/t
kw2ax/qYUDAKBggqhkjOPQQDAgNJADBGAiEA/dVzXiplT/nQGCEOKYYZ0ZYpkLab
NBBMfU/mN47dxG8CIQD1vhxhEXVyL5FdTIR6GCii/mD2IAceYPl2JUI8jS02Nw==
-----END CERTIFICATE-----
)" };
    std::string dsaCertificatePrivateKeyLeft{ R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgNizKWb5B7zV9m+VJ
oyThdFsz48QYtESsKwPBlXH317GhRANCAASPV1vkww5nm6w3htLCccmdqS2bsvHk
4f097QbMndH247z44OQYPoLldS1csr4eLdFuYRYskFH/n+2TDZrH+phQ
-----END PRIVATE KEY-----
)" };
    std::string dsaCertificateRight{ R"(-----BEGIN CERTIFICATE-----
MIIBvzCCAWQCAhI1MAoGCCqGSM49BAMCMIGPMQswCQYDVQQGEwJOTDEUMBIGA1UE
CAwLTmV0aGVybGFuZHMxDDAKBgNVBAcMA1NvbjEQMA4GA1UECgwHUmljaGFyZDEQ
MA4GA1UECwwHUmljaGFyZDEUMBIGA1UEAwwLcnBldGVycy5vcmcxIjAgBgkqhkiG
9w0BCQEWE3JpY2hhcmRAcnBldGVycy5vcmcwHhcNMjUwMzIwMTYzOTA5WhcNMjYw
MzIwMTYzOTA5WjBFMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEh
MB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMFkwEwYHKoZIzj0CAQYI
KoZIzj0DAQcDQgAEj1db5MMOZ5usN4bSwnHJnaktm7Lx5OH9Pe0GzJ3R9uO8+ODk
GD6C5XUtXLK+Hi3RbmEWLJBR/5/tkw2ax/qYUDAKBggqhkjOPQQDAgNJADBGAiEA
7xcmh4F96+2Ue9nYpDc/xAyaS9ok3OvpiFrEjQjPi2kCIQCKkiTjEpgOaHOuq2gY
FIG+UTm+O3R40/iY9h206Tzsug==
-----END CERTIFICATE-----
)" };
    std::string dsaCertificatePrivateKeyRight{ R"(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgNizKWb5B7zV9m+VJ
oyThdFsz48QYtESsKwPBlXH317GhRANCAASPV1vkww5nm6w3htLCccmdqS2bsvHk
4f097QbMndH247z44OQYPoLldS1csr4eLdFuYRYskFH/n+2TDZrH+phQ
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
