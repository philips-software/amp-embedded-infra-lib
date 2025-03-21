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
    std::string rootCaCertificate{
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIICdTCCAhugAwIBAgIULwmcYHDmNNaIjMkxhavqK1YdGvowCgYIKoZIzj0EAwIw\r\n"
        "gY8xCzAJBgNVBAYTAk5MMRQwEgYDVQQIDAtOZXRoZXJsYW5kczEMMAoGA1UEBwwD\r\n"
        "U29uMRAwDgYDVQQKDAdSaWNoYXJkMRAwDgYDVQQLDAdSaWNoYXJkMRQwEgYDVQQD\r\n"
        "DAtycGV0ZXJzLm9yZzEiMCAGCSqGSIb3DQEJARYTcmljaGFyZEBycGV0ZXJzLm9y\r\n"
        "ZzAeFw0yNTAzMjAxNjM0MzhaFw0yNzEyMTUxNjM0MzhaMIGPMQswCQYDVQQGEwJO\r\n"
        "TDEUMBIGA1UECAwLTmV0aGVybGFuZHMxDDAKBgNVBAcMA1NvbjEQMA4GA1UECgwH\r\n"
        "UmljaGFyZDEQMA4GA1UECwwHUmljaGFyZDEUMBIGA1UEAwwLcnBldGVycy5vcmcx\r\n"
        "IjAgBgkqhkiG9w0BCQEWE3JpY2hhcmRAcnBldGVycy5vcmcwWTATBgcqhkjOPQIB\r\n"
        "BggqhkjOPQMBBwNCAASs2aVQCxjuXi/j+P1nEkBq1vBreaTSIU2qJ7Ef5LAn0KSN\r\n"
        "1GrE1YBRSszzs4+E5+0kUNdUiWZlYoCc19H4RwQ8o1MwUTAdBgNVHQ4EFgQUjXoQ\r\n"
        "8n3vMX8IHRWwAXEC5i6tUScwHwYDVR0jBBgwFoAUjXoQ8n3vMX8IHRWwAXEC5i6t\r\n"
        "UScwDwYDVR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNIADBFAiEA7KCumPkfTtiN\r\n"
        "x8A5IXgZsPlL2YWB4TpNgU5MGd50IBsCIEz0JhuYeidumzxnPdf7iNk7SEyXgBvS\r\n"
        "7koJHaaGZIJl\r\n"
        "-----END CERTIFICATE-----\r\n"
    };
    std::string dsaCertificateLeft{
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIICCjCCAa8CAhI0MAoGCCqGSM49BAMCMIGPMQswCQYDVQQGEwJOTDEUMBIGA1UE\r\n"
        "CAwLTmV0aGVybGFuZHMxDDAKBgNVBAcMA1NvbjEQMA4GA1UECgwHUmljaGFyZDEQ\r\n"
        "MA4GA1UECwwHUmljaGFyZDEUMBIGA1UEAwwLcnBldGVycy5vcmcxIjAgBgkqhkiG\r\n"
        "9w0BCQEWE3JpY2hhcmRAcnBldGVycy5vcmcwHhcNMjUwMzIwMTYzNzA2WhcNMjYw\r\n"
        "MzIwMTYzNzA2WjCBjzELMAkGA1UEBhMCTkwxFDASBgNVBAgMC05ldGhlcmxhbmRz\r\n"
        "MQwwCgYDVQQHDANzb24xEDAOBgNVBAoMB1JpY2hhcmQxEDAOBgNVBAsMB1JpY2hh\r\n"
        "cmQxFDASBgNVBAMMC3JwZXRlcnMub3JnMSIwIAYJKoZIhvcNAQkBFhNyaWNoYXJk\r\n"
        "QHJwZXRlcnMub3JnMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEj1db5MMOZ5us\r\n"
        "N4bSwnHJnaktm7Lx5OH9Pe0GzJ3R9uO8+ODkGD6C5XUtXLK+Hi3RbmEWLJBR/5/t\r\n"
        "kw2ax/qYUDAKBggqhkjOPQQDAgNJADBGAiEA/dVzXiplT/nQGCEOKYYZ0ZYpkLab\r\n"
        "NBBMfU/mN47dxG8CIQD1vhxhEXVyL5FdTIR6GCii/mD2IAceYPl2JUI8jS02Nw==\r\n"
        "-----END CERTIFICATE-----\r\n"
    };
    std::string dsaCertificatePrivateKeyLeft{
        "-----BEGIN PRIVATE KEY-----\r\n" //NOSONAR
        "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgNizKWb5B7zV9m+VJ\r\n"
        "oyThdFsz48QYtESsKwPBlXH317GhRANCAASPV1vkww5nm6w3htLCccmdqS2bsvHk\r\n"
        "4f097QbMndH247z44OQYPoLldS1csr4eLdFuYRYskFH/n+2TDZrH+phQ\r\n"
        "-----END PRIVATE KEY-----\r\n"
    };
    std::string dsaCertificateRight{
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIIBvzCCAWQCAhI1MAoGCCqGSM49BAMCMIGPMQswCQYDVQQGEwJOTDEUMBIGA1UE\r\n"
        "CAwLTmV0aGVybGFuZHMxDDAKBgNVBAcMA1NvbjEQMA4GA1UECgwHUmljaGFyZDEQ\r\n"
        "MA4GA1UECwwHUmljaGFyZDEUMBIGA1UEAwwLcnBldGVycy5vcmcxIjAgBgkqhkiG\r\n"
        "9w0BCQEWE3JpY2hhcmRAcnBldGVycy5vcmcwHhcNMjUwMzIwMTYzOTA5WhcNMjYw\r\n"
        "MzIwMTYzOTA5WjBFMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEh\r\n"
        "MB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMFkwEwYHKoZIzj0CAQYI\r\n"
        "KoZIzj0DAQcDQgAEj1db5MMOZ5usN4bSwnHJnaktm7Lx5OH9Pe0GzJ3R9uO8+ODk\r\n"
        "GD6C5XUtXLK+Hi3RbmEWLJBR/5/tkw2ax/qYUDAKBggqhkjOPQQDAgNJADBGAiEA\r\n"
        "7xcmh4F96+2Ue9nYpDc/xAyaS9ok3OvpiFrEjQjPi2kCIQCKkiTjEpgOaHOuq2gY\r\n"
        "FIG+UTm+O3R40/iY9h206Tzsug==\r\n"
        "-----END CERTIFICATE-----\r\n"
    };
    std::string dsaCertificatePrivateKeyRight{
        "-----BEGIN PRIVATE KEY-----\r\n" //NOSONAR
        "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgNizKWb5B7zV9m+VJ\r\n"
        "oyThdFsz48QYtESsKwPBlXH317GhRANCAASPV1vkww5nm6w3htLCccmdqS2bsvHk\r\n"
        "4f097QbMndH247z44OQYPoLldS1csr4eLdFuYRYskFH/n+2TDZrH+phQ\r\n"
        "-----END PRIVATE KEY-----\r\n"
    };
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
    services::EchoOnSesameDiffieHellman::WithCryptoMbedTls echoLeft{ securedLeft, dsaCertificateLeft, dsaCertificatePrivateKeyLeft, rootCaCertificate, randomDataGenerator, serializerFactory, errorPolicy };
    services::EchoOnSesameDiffieHellman::WithCryptoMbedTls echoRight{ securedRight, dsaCertificateRight, dsaCertificatePrivateKeyRight, rootCaCertificate, randomDataGenerator, serializerFactory, errorPolicy };

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
