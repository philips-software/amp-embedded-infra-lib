#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/synchronous_util/SynchronousTracingEchoInstantiationSecured.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/tracer/TracerWithPrefix.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "gmock/gmock.h"

class SynchronousTracingEchoInstantiationSecuredSymmetricKeyTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    services::SerialCommunicationLoopback serial;
    services::SynchronousSesameSecured::KeyMaterial keyMaterial{ { 1, 2 }, { 1, 3 }, { 1, 2 }, { 1, 3 } };

    services::SynchronousSesameSecuredMbedTlsEncryptors encryptorsLeft;
    services::TracerWithPrefix tracerLeft{ "Left ", services::GlobalTracer() };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<1000> leftSerial{ serial.Server() };
    services::MethodSerializerFactory::OnHeap leftSerializerFactory;
    main_::SynchronousTracingEchoOnSesameSecuredSymmetricKey::WithMessageSize<1000> leftEcho{ { leftSerializerFactory, tracerLeft }, encryptorsLeft, leftSerial, keyMaterial, randomDataGenerator };

    services::SynchronousSesameSecuredMbedTlsEncryptors encryptorsRight;
    services::TracerWithPrefix tracerRight{ "Right ", services::GlobalTracer() };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<1000> rightSerial{ serial.Client() };
    services::MethodSerializerFactory::OnHeap rightSerializerFactory;
    main_::SynchronousTracingEchoOnSesameSecuredSymmetricKey::WithMessageSize<1000> rightEcho{ { rightSerializerFactory, tracerRight }, encryptorsRight, rightSerial, keyMaterial, randomDataGenerator };

    services::ServiceStubProxy serviceProxy{ leftEcho.echo };
    testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };
};

TEST_F(SynchronousTracingEchoInstantiationSecuredSymmetricKeyTest, send_message)
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

class SynchronousTracingEchoInstantiationSecuredDiffieHellmanTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    services::SerialCommunicationLoopback serial;

    services::CertificateAndPrivateKey rootCaCertificateMaterial{ services::GenerateRootCertificate(randomDataGenerator) };
    services::EcSecP256r1PrivateKey rootCaPrivateKey{ infra::MakeRange(rootCaCertificateMaterial.privateKey), randomDataGenerator };
    services::CertificateAndPrivateKey leftCertificateMaterial{ services::GenerateDeviceCertificate(rootCaPrivateKey, randomDataGenerator) };
    services::CertificateAndPrivateKey rightCertificateMaterial{ services::GenerateDeviceCertificate(rootCaPrivateKey, randomDataGenerator) };

    services::TracerWithPrefix tracerLeft{ "Left ", services::GlobalTracer() };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<1000> leftSerial{ serial.Server() };
    services::MethodSerializerFactory::OnHeap leftSerializerFactory;
    main_::SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<1000>::WithCryptoMbedTls leftEcho{ leftSerial, leftSerializerFactory,
        services::SynchronousEchoPolicyDiffieHellman::KeyMaterial{ infra::MakeRange(leftCertificateMaterial.certificate), infra::MakeRange(leftCertificateMaterial.privateKey), infra::MakeRange(rootCaCertificateMaterial.certificate) },
        randomDataGenerator, tracerLeft };

    services::TracerWithPrefix tracerRight{ "Right ", services::GlobalTracer() };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<1000> rightSerial{ serial.Client() };
    services::MethodSerializerFactory::OnHeap rightSerializerFactory;
    main_::SynchronousTracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<1000>::WithCryptoMbedTls rightEcho{ rightSerial, rightSerializerFactory,
        services::SynchronousEchoPolicyDiffieHellman::KeyMaterial{ infra::MakeRange(rightCertificateMaterial.certificate), infra::MakeRange(rightCertificateMaterial.privateKey), infra::MakeRange(rootCaCertificateMaterial.certificate) },
        randomDataGenerator, tracerRight };

    services::ServiceStubProxy serviceProxy{ leftEcho.echo };
    testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };
};

TEST_F(SynchronousTracingEchoInstantiationSecuredDiffieHellmanTest, send_message)
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
