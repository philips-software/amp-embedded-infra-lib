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

    infra::BoundedVector<uint8_t>::WithMaxSize<512> CertificateSignedWith(mbedtls_md_type_t digest)
    {
        mbedtls_x509write_cert writer;
        mbedtls_x509write_crt_init(&writer);
        std::array<uint8_t, 16> serial{ 1 };
        mbedtls_x509write_crt_set_version(&writer, MBEDTLS_X509_CRT_VERSION_3);
        mbedtls_x509write_crt_set_serial_raw(&writer, serial.data(), serial.size());
        mbedtls_x509write_crt_set_md_alg(&writer, digest);
        mbedtls_x509write_crt_set_validity(&writer, "20000101000000", "21000101000000");
        mbedtls_x509write_crt_set_subject_name(&writer, "CN=subject");
        mbedtls_x509write_crt_set_subject_key(&writer, &const_cast<mbedtls_pk_context&>(privateKey.Context()));
        mbedtls_x509write_crt_set_issuer_name(&writer, "CN=root");
        mbedtls_x509write_crt_set_issuer_key(&writer, &const_cast<mbedtls_pk_context&>(rootPrivateKey.Context()));

        std::array<uint8_t, 512> buffer{};
        auto size = mbedtls_x509write_crt_der(&writer, buffer.data(), buffer.size(), &services::MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator);
        mbedtls_x509write_crt_free(&writer);
        really_assert(size > 0);

        return infra::BoundedVector<uint8_t>::WithMaxSize<512>(buffer.end() - size, buffer.end());
    }
};

TEST_F(SesameCryptoMbedTlsCertificateTest, sign_and_verify)
{
    std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    auto signature = signer.Sign(infra::MakeRange(data));
    EXPECT_TRUE(verifier.Verify(infra::MakeRange(data), infra::MakeRange(signature.first), infra::MakeRange(signature.second)));
}

TEST_F(SesameCryptoMbedTlsCertificateTest, sign_and_verify_with_sha384_signed_certificate)
{
    auto sha384CertificateDer = CertificateSignedWith(MBEDTLS_MD_SHA384);
    services::EcSecP256r1DsaVerifierMbedTls sha384Verifier{ infra::MakeRange(sha384CertificateDer), infra::MakeRange(rootCertificateDer) };

    std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    auto signature = signer.Sign(infra::MakeRange(data));

    EXPECT_THAT(sha384Verifier.Verify(infra::MakeRange(data), infra::MakeRange(signature.first), infra::MakeRange(signature.second)), testing::IsTrue());
}

TEST_F(SesameCryptoMbedTlsCertificateTest, certificate_signed_with_sha1_is_rejected)
{
    auto sha1CertificateDer = CertificateSignedWith(MBEDTLS_MD_SHA1);
    services::EcSecP256r1DsaVerifierMbedTls sha1Verifier{ infra::MakeRange(sha1CertificateDer), infra::MakeRange(rootCertificateDer) };

    std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    auto signature = signer.Sign(infra::MakeRange(data));

    EXPECT_THAT(sha1Verifier.Verify(infra::MakeRange(data), infra::MakeRange(signature.first), infra::MakeRange(signature.second)), testing::IsFalse());
}
