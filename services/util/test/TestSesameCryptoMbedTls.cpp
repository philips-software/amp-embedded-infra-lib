#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "services/util/MbedTlsRandomDataGeneratorWrapper.hpp"
#include "services/util/SesameCryptoMbedTls.hpp"
#include "services/util/SesameSecured.hpp"
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

TEST_F(SesameCryptoMbedTlsCertificateTest, sign_and_verify_with_sha384_signed_certificate)
{
    mbedtls_x509write_cert sha384CertificateWriter;
    mbedtls_x509write_crt_init(&sha384CertificateWriter);
    std::array<uint8_t, 16> serial{ 1 };
    mbedtls_x509write_crt_set_version(&sha384CertificateWriter, MBEDTLS_X509_CRT_VERSION_3);
    mbedtls_x509write_crt_set_serial_raw(&sha384CertificateWriter, serial.data(), serial.size());
    mbedtls_x509write_crt_set_md_alg(&sha384CertificateWriter, MBEDTLS_MD_SHA384);
    mbedtls_x509write_crt_set_validity(&sha384CertificateWriter, "20000101000000", "21000101000000");
    mbedtls_x509write_crt_set_subject_name(&sha384CertificateWriter, "CN=subject");
    mbedtls_x509write_crt_set_subject_key(&sha384CertificateWriter, &const_cast<mbedtls_pk_context&>(privateKey.Context()));
    mbedtls_x509write_crt_set_issuer_name(&sha384CertificateWriter, "CN=root");
    mbedtls_x509write_crt_set_issuer_key(&sha384CertificateWriter, &const_cast<mbedtls_pk_context&>(rootPrivateKey.Context()));

    std::array<uint8_t, 512> buffer{};
    auto size = mbedtls_x509write_crt_der(&sha384CertificateWriter, buffer.data(), buffer.size(), &services::MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator);
    mbedtls_x509write_crt_free(&sha384CertificateWriter);
    ASSERT_THAT(size, testing::Gt(0));

    services::EcSecP256r1DsaVerifierMbedTls sha384Verifier{ infra::ConstByteRange(buffer.data() + buffer.size() - size, buffer.data() + buffer.size()), infra::MakeRange(rootCertificateDer) };

    std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    auto signature = signer.Sign(infra::MakeRange(data));

    EXPECT_THAT(sha384Verifier.Verify(infra::MakeRange(data), infra::MakeRange(signature.first), infra::MakeRange(signature.second)), testing::IsTrue());
}
