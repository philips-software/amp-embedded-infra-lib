#include "generated/echo/TracingSesameSecurity.pb.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/tracer/TracerWithPrefix.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "services/util/TracingEchoInstantiationSecured.hpp"
#include "services/util/test_doubles/SesameCryptoMbedTlsAdapters.hpp"
#include "gmock/gmock.h"
#include <numeric>

namespace
{
    struct CryptoMbedTls
    {
        CryptoMbedTls(infra::ConstByteRange privateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : keyExchange(randomDataGenerator)
            , signer(privateKey, randomDataGenerator)
        {}

        services::AesGcmEncryptionMbedTlsAdapter sendEncryption;
        services::AesGcmEncryptionMbedTlsAdapter receiveEncryption;
        services::EcSecP256r1DiffieHellmanMbedTlsAdapter keyExchange;
        services::EcSecP256r1DsaSignerMbedTlsAdapter signer;
        services::EcSecP256r1DsaVerifierMbedTlsAdapter verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
        main_::TracingEchoOnSesameSecuredDiffieHellman::Crypto crypto{ sendEncryption, receiveEncryption, { keyExchange, signer, verifier, keyExpander } };
    };
}

class TracingEchoInstantiationSecuredDiffieHellmanTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    services::SerialCommunicationLoopback serial;

    services::EcSecP256r1PrivateKey rootCaPrivateKey{ randomDataGenerator };
    services::EcSecP256r1Certificate rootCaCertificate{ rootCaPrivateKey, "CN=Root", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> rootCaCertificateDer{ rootCaCertificate.Der() };

    services::EcSecP256r1PrivateKey privateKeyLeft{ randomDataGenerator };
    services::EcSecP256r1PrivateKey::DerEncoded privateKeyLeftDer{ privateKeyLeft.Der() };
    services::EcSecP256r1Certificate certificateLeft{ privateKeyLeft, "CN=left", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateLeftDer{ certificateLeft.Der() };

    services::EcSecP256r1PrivateKey privateKeyRight{ randomDataGenerator };
    services::EcSecP256r1PrivateKey::DerEncoded privateKeyRightDer{ privateKeyRight.Der() };
    services::EcSecP256r1Certificate certificateRight{ privateKeyRight, "CN=right", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateRightDer{ certificateRight.Der() };

    CryptoMbedTls cryptoLeft{ infra::MakeRange(privateKeyLeftDer), randomDataGenerator };
    services::TracerWithPrefix tracerLeft{ "Left ", services::GlobalTracer() };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<4000> leftSerial{ serial.Server() };
    services::MethodSerializerFactory::OnHeap leftSerializerFactory;
    main_::TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<4000, 2> leftEcho{ leftSerial, leftSerializerFactory, cryptoLeft.crypto, infra::MakeRange(certificateLeftDer), infra::MakeRange(rootCaCertificateDer), tracerLeft };

    CryptoMbedTls cryptoRight{ infra::MakeRange(privateKeyRightDer), randomDataGenerator };
    services::TracerWithPrefix tracerRight{ "Right ", services::GlobalTracer() };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<4000> rightSerial{ serial.Client() };
    services::MethodSerializerFactory::OnHeap rightSerializerFactory;
    main_::TracingEchoOnSesameSecuredDiffieHellman::WithMessageSize<4000, 2> rightEcho{ rightSerial, rightSerializerFactory, cryptoRight.crypto, infra::MakeRange(certificateRightDer), infra::MakeRange(rootCaCertificateDer), tracerRight };

    services::ServiceStubProxy serviceProxy{ leftEcho.echo };
    testing::StrictMock<services::ServiceStub> service{ rightEcho.echo };

    sesame_security::DiffieHellmanKeyEstablishmentNameTracer diffieHellmanKeyEstablishmentTracerLeft{ leftEcho.echo };
    sesame_security::DiffieHellmanKeyEstablishmentNameTracer diffieHellmanKeyEstablishmentTracerRight{ rightEcho.echo };
};

TEST_F(TracingEchoInstantiationSecuredDiffieHellmanTest, send_message)
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

TEST_F(TracingEchoInstantiationSecuredDiffieHellmanTest, send_multiple_messages)
{
    EXPECT_CALL(service, Method(5)).Times(3).WillRepeatedly([this]()
        {
            service.MethodDone();
        });

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

TEST_F(TracingEchoInstantiationSecuredDiffieHellmanTest, send_message_spanning_multiple_blocks)
{
    std::vector<uint8_t> payload(50);
    std::iota(payload.begin(), payload.end(), 0);

    EXPECT_CALL(service, MethodBytes(testing::ElementsAreArray(payload))).WillOnce(testing::Invoke([this]()
        {
            service.MethodDone();
        }));

    serviceProxy.RequestSend([this, &payload]()
        {
            serviceProxy.MethodBytes(infra::MakeRange(payload));
        });

    ExecuteAllActions();
}
