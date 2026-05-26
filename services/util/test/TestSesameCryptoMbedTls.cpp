#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "services/util/SesameCryptoMbedTls.hpp"
#include "gmock/gmock.h"

class SesameCryptoMbedTlsPublicPrivateKeyTest
    : public testing::Test
{
public:
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

    services::EcSecP256r1PrivateKey privateKey{ randomDataGenerator };
    services::EcSecP256r1PublicKey publicKey{ privateKey, randomDataGenerator };

    services::EcSecP256r1DsaSignerMbedTls signer{ privateKey.Der(), randomDataGenerator };
    services::EcSecP256r1DsaVerifierMbedTls verifier{ publicKey.Der() };
};

TEST_F(SesameCryptoMbedTlsPublicPrivateKeyTest, sign_and_verify)
{
    std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    auto signature = signer.Sign(infra::MakeRange(data));
    EXPECT_TRUE(verifier.Verify(infra::MakeRange(data), infra::MakeRange(signature.first), infra::MakeRange(signature.second)));
}

class SesameCryptoMbedTlsCertificateTest
    : public testing::Test
{
public:
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;

    services::EcSecP256r1PrivateKey rootPrivateKey{ randomDataGenerator };
    services::EcSecP256r1Certificate rootCertificate{ rootPrivateKey, "CN=root", rootPrivateKey, "CN=root", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> rootCertificateDer{ rootCertificate.Der() };
    services::EcSecP256r1PrivateKey privateKey{ randomDataGenerator };
    services::EcSecP256r1Certificate certificate{ privateKey, "CN=subject", rootPrivateKey, "CN=issuer", randomDataGenerator };
    infra::BoundedVector<uint8_t>::WithMaxSize<512> certificateDer{ certificate.Der() };

    services::EcSecP256r1DsaSignerMbedTls signer{ privateKey.Der(), randomDataGenerator };
    services::EcSecP256r1DsaVerifierMbedTls verifier{ infra::MakeRange(certificateDer), infra::MakeRange(rootCertificateDer) };
};

TEST_F(SesameCryptoMbedTlsCertificateTest, sign_and_verify)
{
    std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    auto signature = signer.Sign(infra::MakeRange(data));
    EXPECT_TRUE(verifier.Verify(infra::MakeRange(data), infra::MakeRange(signature.first), infra::MakeRange(signature.second)));
}
