#include "services/util/SesameCryptoMbedTls.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "mbedtls/ecdsa.h"
#include "mbedtls/hmac_drbg.h"
#include "mbedtls/sha256.h"
#include "services/util/MbedTlsRandomDataGeneratorWrapper.hpp"

namespace services
{
    namespace
    {
        template<std::size_t Size>
        std::array<uint8_t, Size> ConvertToBytes(const mbedtls_ecp_group& group, const mbedtls_ecp_point& x)
        {
            std::array<uint8_t, Size> result;
            std::size_t size = 0;
            really_assert(mbedtls_ecp_point_write_binary(&group, &x, MBEDTLS_ECP_PF_UNCOMPRESSED, &size, result.data(), result.size()) == 0);
            really_assert(size == result.size());
            return result;
        }

        template<std::size_t Size>
        std::array<uint8_t, Size> ConvertToBytes(mbedtls_mpi& x)
        {
            std::array<uint8_t, Size> result;
            really_assert(mbedtls_mpi_write_binary(&x, result.data(), result.size()) == 0);
            return result;
        }
    }

    EcSecP256r1DiffieHellmanMbedTls::EcSecP256r1DiffieHellmanMbedTls(hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : randomDataGenerator(randomDataGenerator)
    {
        mbedtls_ecp_group_init(&group);
        really_assert(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1) == 0);

        mbedtls_mpi_init(&privateKey);
        mbedtls_ecp_point_init(&publicKey);

        really_assert(mbedtls_ecdh_gen_public(&group, &privateKey, &publicKey, &MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);
    }

    EcSecP256r1DiffieHellmanMbedTls::~EcSecP256r1DiffieHellmanMbedTls()
    {
        mbedtls_ecp_point_free(&publicKey);
        mbedtls_mpi_free(&privateKey);
    }

    std::array<uint8_t, 65> EcSecP256r1DiffieHellmanMbedTls::PublicKey() const
    {
        return ConvertToBytes<65>(group, publicKey);
    }

    std::array<uint8_t, 32> EcSecP256r1DiffieHellmanMbedTls::SharedSecret(infra::ConstByteRange otherPublicKey) const
    {
        mbedtls_mpi z;
        mbedtls_mpi_init(&z);

        mbedtls_ecp_point dhOtherPublicKey;
        mbedtls_ecp_point_init(&dhOtherPublicKey);
        really_assert(mbedtls_ecp_point_read_binary(&group, &dhOtherPublicKey, otherPublicKey.begin(), otherPublicKey.size()) == 0);

        really_assert(mbedtls_ecdh_compute_shared(const_cast<mbedtls_ecp_group*>(&group), &z, &dhOtherPublicKey, &privateKey, &MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);

        auto sharedSecret = ConvertToBytes<32>(z);

        mbedtls_ecp_point_free(&dhOtherPublicKey);
        mbedtls_mpi_free(&z);

        return sharedSecret;
    }

    EcSecP256r1DsaSignerMbedTls::EcSecP256r1DsaSignerMbedTls(infra::ConstByteRange dsaCertificatePrivateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : randomDataGenerator(randomDataGenerator)
    {
        mbedtls_ecp_group_init(&group);
        really_assert(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1) == 0);

        mbedtls_ecdh_init(&context);
        really_assert(mbedtls_ecdh_setup(&context, group.id) == 0);

        mbedtls_mpi_init(&privateKey);
        mbedtls_ecp_point dsaPublicKey;
        mbedtls_ecp_point_init(&dsaPublicKey);

        mbedtls_pk_context dsaPrivateKeyContext;
        mbedtls_pk_init(&dsaPrivateKeyContext);
        really_assert(mbedtls_pk_parse_key(&dsaPrivateKeyContext, dsaCertificatePrivateKey.begin(), dsaCertificatePrivateKey.size(), nullptr, 0, &MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);
        really_assert(mbedtls_ecp_export(mbedtls_pk_ec(dsaPrivateKeyContext), &group, &privateKey, &dsaPublicKey) == 0);
        mbedtls_pk_free(&dsaPrivateKeyContext);
        mbedtls_ecp_point_free(&dsaPublicKey);
    }

    EcSecP256r1DsaSignerMbedTls::~EcSecP256r1DsaSignerMbedTls()
    {
        mbedtls_mpi_free(&privateKey);
        mbedtls_ecdh_free(&context);
        mbedtls_ecp_group_free(&group);
    }

    std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> EcSecP256r1DsaSignerMbedTls::Sign(infra::ConstByteRange data) const
    {
        std::array<uint8_t, 32> hash;

        auto result = mbedtls_sha256(data.begin(), data.size(), hash.data(), 0);
        assert(result == 0);

        mbedtls_mpi r;
        mbedtls_mpi_init(&r);
        mbedtls_mpi s;
        mbedtls_mpi_init(&s);

        really_assert(mbedtls_ecdsa_sign(const_cast<mbedtls_ecp_group*>(&group), &r, &s, &privateKey, hash.data(), hash.size(), &MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);

        auto encodedR = ConvertToBytes<32>(r);
        auto encodedS = ConvertToBytes<32>(s);

        mbedtls_mpi_free(&s);
        mbedtls_mpi_free(&r);

        return { encodedR, encodedS };
    }

    EcSecP256r1DsaVerifierMbedTls::EcSecP256r1DsaVerifierMbedTls(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)
    {
        mbedtls_x509_crt_init(&rootCertificate);
        really_assert(mbedtls_x509_crt_parse(&rootCertificate, rootCaCertificate.begin(), rootCaCertificate.size()) == 0);
        really_assert(mbedtls_pk_get_type(&rootCertificate.pk) == MBEDTLS_PK_ECKEY);

        mbedtls_ecp_group_init(&group);
        really_assert(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1) == 0);

        mbedtls_ecp_point_init(&publicKey);

        mbedtls_ecp_group otherGroup;
        mbedtls_mpi otherDsaPrivateKey;
        mbedtls_ecp_group_init(&otherGroup);
        mbedtls_mpi_init(&otherDsaPrivateKey);

        mbedtls_x509_crt certificate;
        mbedtls_x509_crt_init(&certificate);

        really_assert(mbedtls_x509_crt_parse(&certificate, dsaCertificate.begin(), dsaCertificate.size()) == 0);
        really_assert(mbedtls_pk_get_type(&certificate.pk) == MBEDTLS_PK_ECKEY);

        std::array<uint8_t, 32> hash;
        const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md(mdInfo, certificate.tbs.p, certificate.tbs.len, hash.data());
        valid = mbedtls_pk_verify(&rootCertificate.pk, MBEDTLS_MD_SHA256, hash.data(), hash.size(), certificate.MBEDTLS_PRIVATE(sig).p, certificate.MBEDTLS_PRIVATE(sig).len) == 0;

        really_assert(mbedtls_ecp_export(mbedtls_pk_ec(certificate.pk), &otherGroup, &otherDsaPrivateKey, &publicKey) == 0);

        mbedtls_x509_crt_free(&certificate);

        mbedtls_mpi_free(&otherDsaPrivateKey);
        mbedtls_ecp_group_free(&otherGroup);
    }

    EcSecP256r1DsaVerifierMbedTls::~EcSecP256r1DsaVerifierMbedTls()
    {
        mbedtls_ecp_point_free(&publicKey);
        mbedtls_ecp_group_free(&group);
        mbedtls_x509_crt_free(&rootCertificate);
    }

    bool EcSecP256r1DsaVerifierMbedTls::Verify(infra::ConstByteRange data, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS) const
    {
        if (!valid)
            return false;

        std::array<uint8_t, 32> hash;

        auto hashResult = mbedtls_sha256(data.begin(), data.size(), hash.data(), 0);
        assert(hashResult == 0);

        mbedtls_mpi r;
        mbedtls_mpi_init(&r);
        mbedtls_mpi s;
        mbedtls_mpi_init(&s);

        really_assert(mbedtls_mpi_read_binary(&r, signatureR.begin(), signatureR.size()) == 0);
        really_assert(mbedtls_mpi_read_binary(&s, signatureS.begin(), signatureS.size()) == 0);

        bool result = mbedtls_ecdsa_verify(const_cast<mbedtls_ecp_group*>(&group), hash.data(), hash.size(), &publicKey, &r, &s) == 0;

        mbedtls_mpi_free(&s);
        mbedtls_mpi_free(&r);

        return result;
    }

    void HmacDrbgSha256MbedTls::Expand(infra::ConstByteRange seed, infra::ByteRange expandedMaterial) const
    {
        mbedtls_hmac_drbg_context context;
        mbedtls_hmac_drbg_init(&context);

        really_assert(mbedtls_hmac_drbg_seed_buf(&context, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), seed.begin(), seed.size()) == 0);
        really_assert(mbedtls_hmac_drbg_random(&context, expandedMaterial.begin(), expandedMaterial.size()) == 0);

        mbedtls_hmac_drbg_free(&context);
    }

    AesGcmEncryptionMbedTls::AesGcmEncryptionMbedTls()
    {
        mbedtls_gcm_init(&context);
    }

    AesGcmEncryptionMbedTls::~AesGcmEncryptionMbedTls()
    {
        mbedtls_gcm_free(&context);
    }

    void AesGcmEncryptionMbedTls::EncryptWithKey(infra::ConstByteRange key)
    {
        encrypt = true;
        mbedtls_gcm_setkey(&context, MBEDTLS_CIPHER_ID_AES, key.begin(), key.size() * 8); //NOSONAR
    }

    void AesGcmEncryptionMbedTls::DecryptWithKey(infra::ConstByteRange key)
    {
        encrypt = false;
        mbedtls_gcm_setkey(&context, MBEDTLS_CIPHER_ID_AES, key.begin(), key.size() * 8); //NOSONAR
    }

    void AesGcmEncryptionMbedTls::Start(infra::ConstByteRange iv)
    {
        really_assert(mbedtls_gcm_starts(&context, encrypt ? MBEDTLS_GCM_ENCRYPT : MBEDTLS_GCM_DECRYPT, iv.begin(), iv.size()) == 0);
    }

    std::size_t AesGcmEncryptionMbedTls::Update(infra::ConstByteRange from, infra::ByteRange to)
    {
        std::size_t processedSize = 0;
        really_assert(mbedtls_gcm_update(&context, from.begin(), from.size(), to.begin(), to.size(), &processedSize) == 0);
        return processedSize;
    }

    std::size_t AesGcmEncryptionMbedTls::Finish(infra::ByteRange to, infra::ByteRange mac)
    {
        std::size_t processedSize = 0;
        really_assert(mbedtls_gcm_finish(&context, to.begin(), to.size(), &processedSize, mac.begin(), mac.size()) == 0);
        return processedSize;
    }

    EcSecP256r1PrivateKey::EcSecP256r1PrivateKey(hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        mbedtls_pk_init(&context);
        really_assert(mbedtls_pk_setup(&context, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) == 0);
        really_assert(mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, mbedtls_pk_ec(context), &services::MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);
    }

    EcSecP256r1PrivateKey::EcSecP256r1PrivateKey(infra::ConstByteRange key, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        mbedtls_pk_init(&context);
        really_assert(mbedtls_pk_setup(&context, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) == 0);
        really_assert(mbedtls_pk_parse_key(&context, key.begin(), key.size(), nullptr, 0, &services::MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);
    }

    EcSecP256r1PrivateKey::~EcSecP256r1PrivateKey()
    {
        mbedtls_pk_free(&context);
    }

    infra::BoundedString::WithStorage<228> EcSecP256r1PrivateKey::Pem() const
    {
        infra::BoundedString::WithStorage<228> result(228, '\0');
        really_assert(mbedtls_pk_write_key_pem(&context, reinterpret_cast<unsigned char*>(result.data()), result.size()) == 0);
        really_assert(result.find('\0') == result.size() - 1);
        return result;
    }

    std::array<uint8_t, 121> EcSecP256r1PrivateKey::Der() const
    {
        std::array<uint8_t, 121> result{};
        auto size = mbedtls_pk_write_key_der(&context, result.data(), result.size());
        really_assert(size == result.size());
        return result;
    }

    const mbedtls_pk_context& EcSecP256r1PrivateKey::Context() const
    {
        return context;
    }

    EcSecP256r1Certificate::EcSecP256r1Certificate(const EcSecP256r1PrivateKey& subjectKey, const char* subjectName, const EcSecP256r1PrivateKey& issuerKey, const char* issuerName, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : randomDataGenerator(randomDataGenerator)
    {
        mbedtls_x509write_crt_init(&dsaCertificate);

        std::array<uint8_t, 16> serial;
        randomDataGenerator.GenerateRandomData(infra::MakeRange(serial));
        mbedtls_x509write_crt_set_version(&dsaCertificate, MBEDTLS_X509_CRT_VERSION_3);
        mbedtls_x509write_crt_set_serial_raw(&dsaCertificate, serial.data(), serial.size());
        mbedtls_x509write_crt_set_md_alg(&dsaCertificate, MBEDTLS_MD_SHA256);
        mbedtls_x509write_crt_set_validity(&dsaCertificate, "20000101000000", "21000101000000");
        mbedtls_x509write_crt_set_subject_name(&dsaCertificate, subjectName);
        mbedtls_x509write_crt_set_subject_key(&dsaCertificate, &const_cast<mbedtls_pk_context&>(subjectKey.Context()));
        mbedtls_x509write_crt_set_issuer_name(&dsaCertificate, issuerName);
        mbedtls_x509write_crt_set_issuer_key(&dsaCertificate, &const_cast<mbedtls_pk_context&>(issuerKey.Context()));
        mbedtls_x509write_crt_set_basic_constraints(&dsaCertificate, 0, -1);
    }

    EcSecP256r1Certificate::~EcSecP256r1Certificate()
    {
        mbedtls_x509write_crt_free(&dsaCertificate);
    }

    infra::BoundedString::WithStorage<512> EcSecP256r1Certificate::Pem() const
    {
        infra::BoundedString::WithStorage<512> result(512, '\0');
        really_assert(mbedtls_x509write_crt_pem(&const_cast<mbedtls_x509write_cert&>(dsaCertificate), reinterpret_cast<unsigned char*>(const_cast<char*>(result.data())), result.size(), &services::MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator) == 0);
        result.resize(result.find('\0') + 1);
        return result;
    }

    infra::BoundedVector<uint8_t>::WithMaxSize<512> EcSecP256r1Certificate::Der() const
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<512> result(512, static_cast<uint8_t>(0));
        auto size = mbedtls_x509write_crt_der(&const_cast<mbedtls_x509write_cert&>(dsaCertificate), result.data(), result.size(), &services::MbedTlsRandomDataGeneratorWrapper, &randomDataGenerator);
        result.erase(result.begin(), result.begin() + result.size() - size);
        return result;
    }
}
