#include "generated/echo/TracingSesameSecurity.pb.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/tracer/TracerWithPrefix.hpp"
#include "services/tracer/TracingEchoInstantiationSecured.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "gmock/gmock.h"

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class EchoInstantiationSecuredSymmetrickey
    {
    public:
        hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        services::SerialCommunicationLoopback serial;

        services::SesameSecured::KeyType keyA{ 1, 2 };
        services::SesameSecured::KeyType keyB{ 3, 4 };
        services::SesameSecured::IvType ivA{ 5, 6 };
        services::SesameSecured::IvType ivB{ 7, 8 };
        services::SesameSecured::KeyMaterial keyMaterialLeft{ keyA, ivA, keyB, ivB };
        services::SesameSecured::KeyMaterial keyMaterialRight{ keyB, ivB, keyA, ivA };

        constexpr static std::size_t leftSize = LeftSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<leftSize> leftSerial{ serial.Server() };
        services::MethodSerializerFactory::OnHeap leftSerializerFactory;
        main_::EchoOnSesameSecuredSymmetricKey::WithMessageSize<leftSize, 2> leftEcho{ leftSerial, leftSerializerFactory, keyMaterialLeft, randomDataGenerator };

        constexpr static std::size_t rightSize = RightSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<rightSize> rightSerial{ serial.Client() };
        services::MethodSerializerFactory::OnHeap rightSerializerFactory;
        main_::EchoOnSesameSecuredSymmetricKey::WithMessageSize<rightSize, 2> rightEcho{ rightSerial, rightSerializerFactory, keyMaterialRight, randomDataGenerator };

        services::ServiceStubProxy serviceProxy{ leftEcho.echo };
        testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };
    };
}

class EchoInstantiationSecuredSymmetricKeyTest
    : public testing::Test
    , public infra::ClockFixture
    , public EchoInstantiationSecuredSymmetrickey<256, 1024>
{};


 TEST_F(EchoInstantiationSecuredSymmetricKeyTest, send_message)
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

 TEST_F(EchoInstantiationSecuredSymmetricKeyTest, send_multiple_messages)
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

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class EchoInstantiationSecuredDiffieHellman
    {
    public:
        hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        services::SerialCommunicationLoopback serial;

        services::CertificateAndPrivateKey rootCaCertificateMaterial{ services::GenerateRootCertificate(randomDataGenerator) };
        services::EcSecP256r1PrivateKey rootCaPrivateKey{ randomDataGenerator };
        services::EcSecP256r1Certificate rootCaCertificate{ rootCaPrivateKey, "CN=Root", rootCaPrivateKey, "CN=Root", randomDataGenerator };
        std::string rootCaCertificatePem{ infra::AsStdString(rootCaCertificate.Pem()) };
        infra::BoundedVector<uint8_t>::WithMaxSize<512> rootCaCertificateDer{ rootCaCertificate.Der() };

        services::CertificateAndPrivateKey deviceCertificateMaterial{ services::GenerateDeviceCertificate(rootCaPrivateKey, randomDataGenerator) };
        services::EcSecP256r1PrivateKey privateKeyLeft{ randomDataGenerator };
        std::string privateKeyLeftPem{ infra::AsStdString(privateKeyLeft.Pem()) };
        services::EcSecP256r1PrivateKey::DerEncoded privateKeyLeftDer{ privateKeyLeft.Der() };
        services::EcSecP256r1Certificate certificateLeft{ privateKeyLeft, "CN=left", rootCaPrivateKey, "CN=Root", randomDataGenerator };
        std::string certificateLeftPem{ infra::AsStdString(certificateLeft.Pem()) };
        infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateLeftDer{ certificateLeft.Der() };

        services::EcSecP256r1PrivateKey privateKeyRight{ randomDataGenerator };
        std::string privateKeyRightPem{ infra::AsStdString(privateKeyRight.Pem()) };
        services::EcSecP256r1PrivateKey::DerEncoded privateKeyRightDer{ privateKeyRight.Der() };
        services::EcSecP256r1Certificate certificateRight{ privateKeyRight, "CN=right", rootCaPrivateKey, "CN=Root", randomDataGenerator };
        std::string certificateRightPem{ infra::AsStdString(certificateRight.Pem()) };
        infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateRightDer{ certificateRight.Der() };

        services::EchoPolicyDiffieHellman::KeyMaterial keyMaterialLeft{ infra::MakeRange(certificateLeftDer), infra::MakeRange(privateKeyLeftDer), infra::MakeRange(rootCaCertificateDer) };
        services::EchoPolicyDiffieHellman::KeyMaterial keyMaterialRight{ infra::MakeRange(certificateRightDer), infra::MakeRange(privateKeyRightDer), infra::MakeRange(rootCaCertificateDer) };

        services::TracerWithPrefix tracerLeft{ "Left ", services::GlobalTracer() };
        constexpr static std::size_t leftSize = LeftSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<leftSize> leftSerial{ serial.Server() };
        services::MethodSerializerFactory::OnHeap leftSerializerFactory;
        typename main_::TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<leftSize, 2>::WithCryptoMbedTls leftEcho{ leftSerial, leftSerializerFactory, keyMaterialLeft, randomDataGenerator, tracerLeft };

        services::TracerWithPrefix tracerRight{ "Right                                                      ", services::GlobalTracer() };
        constexpr static std::size_t rightSize = RightSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<rightSize> rightSerial{ serial.Client() };
        services::MethodSerializerFactory::OnHeap rightSerializerFactory;
        typename main_::TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<rightSize, 2>::WithCryptoMbedTls rightEcho{ rightSerial, rightSerializerFactory, keyMaterialRight, randomDataGenerator, tracerRight };

        services::ServiceStubProxy serviceProxy{ leftEcho.echo };
        testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };

        sesame_security::DiffieHellmanKeyEstablishmentNameTracer diffieHellmanKeyEstablishmentTracerLeft{ leftEcho.echo };
        sesame_security::DiffieHellmanKeyEstablishmentNameTracer diffieHellmanKeyEstablishmentTracerRight{ rightEcho.echo };
    };
}

class EchoInstantiationSecuredDiffieHellmanTest
    : public testing::Test
    , public infra::ClockFixture
    , public EchoInstantiationSecuredDiffieHellman<4000, 4000>
{};

TEST_F(EchoInstantiationSecuredDiffieHellmanTest, send_message)
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

TEST_F(EchoInstantiationSecuredDiffieHellmanTest, send_multiple_messages)
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
