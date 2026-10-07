#include "hal/synchronous_interfaces/test_doubles/SynchronousRandomDataGeneratorMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/util/EchoPolicyDiffieHellman.hpp"
#include "services/util/test_doubles/SesameCryptoMbedTlsAdapters.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"
#include <list>

namespace
{
    class EchoPolicyDiffieHellmanMock
        : public services::EchoPolicyDiffieHellman
    {
    public:
        using services::EchoPolicyDiffieHellman::EchoPolicyDiffieHellman;

        MOCK_METHOD(void, KeyExchangeSuccessful, (), (override));
        MOCK_METHOD(void, KeyExchangeFailed, (), (override));
    };

    class CountingDiffieHellman
        : public services::EcSecP256r1DiffieHellman
    {
    public:
        explicit CountingDiffieHellman(services::EcSecP256r1DiffieHellman& delegate)
            : delegate(delegate)
        {}

        void GenerateKeyPair(const infra::Function<void(const std::array<uint8_t, 65>& publicKey)>& onDone) override
        {
            ++generateKeyPairCount;
            delegate.GenerateKeyPair(onDone);
        }

        void SharedSecret(infra::ConstByteRange otherPublicKey, const infra::Function<void(const std::array<uint8_t, 32>& sharedSecret)>& onDone) override
        {
            delegate.SharedSecret(otherPublicKey, onDone);
        }

        std::size_t generateKeyPairCount = 0;

    private:
        services::EcSecP256r1DiffieHellman& delegate;
    };

    struct Side
    {
        Side(services::Sesame& lower, services::MethodSerializerFactory& serializerFactory, const services::EchoErrorPolicy& errorPolicy, infra::ConstByteRange certificate, infra::ConstByteRange privateKey,
            infra::ConstByteRange rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : keyExchange(randomDataGenerator)
            , signer(privateKey, randomDataGenerator)
            , secured(lower, sendEncryption, receiveEncryption, services::SesameSecured::KeyMaterial{ key, iv, key, iv })
            , echo(secured, serializerFactory, errorPolicy)
            , policy(services::EchoPolicyDiffieHellman::Crypto{ countingKeyExchange, signer, verifier, keyExpander }, echo, echo, secured, certificate, rootCaCertificate)
        {}

        services::SesameSecured::KeyType key{ 1, 2 };
        services::SesameSecured::IvType iv{ 1, 3 };
        services::AesGcmEncryptionMbedTlsAdapter sendEncryption;
        services::AesGcmEncryptionMbedTlsAdapter receiveEncryption;
        services::EcSecP256r1DiffieHellmanMbedTlsAdapter keyExchange;
        CountingDiffieHellman countingKeyExchange{ keyExchange };
        services::EcSecP256r1DsaSignerMbedTlsAdapter signer;
        services::EcSecP256r1DsaVerifierMbedTlsAdapter verifier;
        services::HmacDrbgSha256MbedTls keyExpander;
        services::SesameSecured::WithBuffers<100> secured;
        services::EchoOnSesame echo;
        testing::StrictMock<EchoPolicyDiffieHellmanMock> policy;
    };
}

class EchoPolicyDiffieHellmanTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    EchoPolicyDiffieHellmanTest()
    {
        EXPECT_CALL(lowerLeft, MaxSendMessageSize()).WillRepeatedly(testing::Return(10000));
        EXPECT_CALL(lowerRight, MaxSendMessageSize()).WillRepeatedly(testing::Return(10000));

        EXPECT_CALL(lowerLeft, RequestSendMessage(testing::_)).WillRepeatedly(testing::Invoke([this]()
            {
                really_assert(!lowerLeftRequest);
                lowerLeftRequest = true;
            }));

        EXPECT_CALL(lowerRight, RequestSendMessage(testing::_)).WillRepeatedly(testing::Invoke([this]()
            {
                really_assert(!lowerRightRequest);
                lowerRightRequest = true;
            }));
    }

    void Initialized()
    {
        EXPECT_CALL(lowerLeft, ResetReading());
        lowerLeft.GetObserver().Initialized();
        EXPECT_CALL(lowerRight, ResetReading());
        lowerRight.GetObserver().Initialized();
    }

    void ExchangeData()
    {
        ExecuteAllActions();

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
        auto& sentData = sentMessages.emplace_back();

        infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
        x.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));
        ExecuteAllActions();
        ASSERT_THAT(writer.Allocatable(), testing::IsTrue());

        y.GetObserver().ReceivedMessage(readers.emplace_back().Emplace(sentData));
        ExecuteAllActions();
    }

    std::list<std::vector<uint8_t>> sentMessages;
    std::list<infra::SharedOptional<infra::StdVectorInputStreamReader>> readers;
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    testing::StrictMock<hal::SynchronousRandomDataGeneratorMock> randomDataGenerator;
    infra::Execute expectRandomData{ [this]()
        {
            EXPECT_CALL(randomDataGenerator, GenerateRandomData(testing::_)).WillRepeatedly(testing::Invoke([](infra::ByteRange data)
                {
                    static uint8_t fill = 0;
                    std::fill(data.begin(), data.end(), fill++);
                }));
        } };
    testing::StrictMock<services::SesameMock> lowerLeft;
    testing::StrictMock<services::SesameMock> lowerRight;
    bool lowerLeftRequest = false;
    bool lowerRightRequest = false;

    services::EcSecP256r1PrivateKey rootCaPrivateKey{ randomDataGenerator };
    services::EcSecP256r1Certificate rootCaCertificate{ rootCaPrivateKey, "CN=Root", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> rootCaCertificateDer{ rootCaCertificate.Der() };

    services::EcSecP256r1PrivateKey otherRootCaPrivateKey{ randomDataGenerator };

    services::EcSecP256r1PrivateKey privateKeyLeft{ randomDataGenerator };
    services::EcSecP256r1PrivateKey::DerEncoded privateKeyLeftDer{ privateKeyLeft.Der() };
    services::EcSecP256r1Certificate certificateLeft{ privateKeyLeft, "CN=left", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateLeftDer{ certificateLeft.Der() };

    services::EcSecP256r1PrivateKey privateKeyRight{ randomDataGenerator };
    services::EcSecP256r1PrivateKey::DerEncoded privateKeyRightDer{ privateKeyRight.Der() };
    services::EcSecP256r1Certificate certificateRight{ privateKeyRight, "CN=right", rootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateRightDer{ certificateRight.Der() };
    services::EcSecP256r1Certificate certificateRightFromOtherRoot{ privateKeyRight, "CN=right", otherRootCaPrivateKey, "CN=Root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateRightFromOtherRootDer{ certificateRightFromOtherRoot.Der() };

    services::MethodSerializerFactory::ForServices<services::ServiceStub, sesame_security::DiffieHellmanKeyEstablishment>::AndProxies<services::ServiceStubProxy, sesame_security::DiffieHellmanKeyEstablishmentProxy> serializerFactoryLeft;
    services::MethodSerializerFactory::ForServices<services::ServiceStub, sesame_security::DiffieHellmanKeyEstablishment>::AndProxies<services::ServiceStubProxy, sesame_security::DiffieHellmanKeyEstablishmentProxy> serializerFactoryRight;

    Side left{ lowerLeft, serializerFactoryLeft, errorPolicy, infra::MakeRange(certificateLeftDer), infra::MakeRange(privateKeyLeftDer), infra::MakeRange(rootCaCertificateDer), randomDataGenerator };
};

class EchoPolicyDiffieHellmanWithValidPeerTest
    : public EchoPolicyDiffieHellmanTest
{
public:
    EchoPolicyDiffieHellmanWithValidPeerTest()
    {
        Initialized();
    }

    void Reset()
    {
        lowerLeftRequest = false;
        lowerRightRequest = false;
        EXPECT_CALL(lowerLeft, ResetReading());
        EXPECT_CALL(lowerLeft, Reset());
        left.echo.Reset();
        EXPECT_CALL(lowerRight, ResetReading());
        EXPECT_CALL(lowerRight, Reset());
        right.echo.Reset();
    }

    Side right{ lowerRight, serializerFactoryRight, errorPolicy, infra::MakeRange(certificateRightDer), infra::MakeRange(privateKeyRightDer), infra::MakeRange(rootCaCertificateDer), randomDataGenerator };
    services::ServiceStubProxy serviceProxy{ left.echo };
    testing::StrictMock<services::ServiceStub> service{ right.echo };
};

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, key_exchange_succeeds_and_message_is_sent_with_exchanged_keys)
{
    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());

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

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, initialize_while_initializing_starts_over)
{
    Initialized();

    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());

    ExchangeData();
}

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, reset_and_initialize_while_crypto_is_pending_starts_over)
{
    Reset();
    Initialized();

    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());

    ExchangeData();
}

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, initialize_during_certificate_verification_starts_over)
{
    ExecuteAllActions();
    lowerLeftRequest = false;
    std::vector<uint8_t> sentData;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
    lowerLeft.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));
    ExecuteAllActions();

    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    lowerRight.GetObserver().ReceivedMessage(reader.Emplace(sentData));
    ExecuteFirstAction();

    Initialized();

    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());

    ExchangeData();
}

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, key_pair_is_generated_before_initialization_and_again_after_key_exchange)
{
    EXPECT_THAT(left.countingKeyExchange.generateKeyPairCount, testing::Eq(1));

    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());
    ExchangeData();

    EXPECT_THAT(left.countingKeyExchange.generateKeyPairCount, testing::Eq(2));
}

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, next_key_exchange_uses_prepared_key_pair)
{
    EXPECT_CALL(left.policy, KeyExchangeSuccessful()).Times(2);
    EXPECT_CALL(right.policy, KeyExchangeSuccessful()).Times(2);
    ExchangeData();

    Reset();
    Initialized();
    EXPECT_THAT(left.countingKeyExchange.generateKeyPairCount, testing::Eq(2));

    ExchangeData();
}

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, reset_after_exchange_was_sent_generates_new_key_pair)
{
    EXPECT_CALL(right.policy, KeyExchangeSuccessful()).Times(2);

    ExecuteAllActions();
    while (lowerLeftRequest)
        ExchangeMessage(lowerLeftRequest, lowerLeft, lowerRight);

    Reset();
    EXPECT_THAT(left.countingKeyExchange.generateKeyPairCount, testing::Eq(2));

    Initialized();
    EXPECT_THAT(left.countingKeyExchange.generateKeyPairCount, testing::Eq(2));

    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    ExchangeData();
}

TEST_F(EchoPolicyDiffieHellmanWithValidPeerTest, reset_before_exchange_was_sent_keeps_key_pair)
{
    ExecuteAllActions();

    Reset();
    Initialized();
    EXPECT_THAT(left.countingKeyExchange.generateKeyPairCount, testing::Eq(1));

    EXPECT_CALL(left.policy, KeyExchangeSuccessful());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());
    ExchangeData();
}

class EchoPolicyDiffieHellmanWithUntrustedPeerTest
    : public EchoPolicyDiffieHellmanTest
{
public:
    EchoPolicyDiffieHellmanWithUntrustedPeerTest()
    {
        Initialized();
    }

    Side right{ lowerRight, serializerFactoryRight, errorPolicy, infra::MakeRange(certificateRightFromOtherRootDer), infra::MakeRange(privateKeyRightDer), infra::MakeRange(rootCaCertificateDer), randomDataGenerator };
};

TEST_F(EchoPolicyDiffieHellmanWithUntrustedPeerTest, key_exchange_fails_on_side_receiving_untrusted_certificate)
{
    EXPECT_CALL(left.policy, KeyExchangeFailed());
    EXPECT_CALL(right.policy, KeyExchangeSuccessful());

    ExchangeData();
}
