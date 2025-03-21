#include "services/util/EchoOnSesameDiffieHellman.hpp"
#include "mbedtls/ecdsa.h"
#include "mbedtls/hmac_drbg.h"

namespace services
{
    namespace
    {
        template<std::size_t Size>
        std::array<uint8_t, Size> ConvertToBytes(const mbedtls_ecp_group& group, const mbedtls_ecp_point& x)
        {
            std::array<uint8_t, 65> result;
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

    EcSecP256r1DiffieHellman::EcSecP256r1DiffieHellman(hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        mbedtls_ecp_group_init(&group);
        really_assert(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1) == 0);

        mbedtls_mpi_init(&privateKey);
        mbedtls_ecp_point_init(&publicKey);

        rng = &randomDataGenerator;
        really_assert(mbedtls_ecdh_gen_public(&group, &privateKey, &publicKey, &EcSecP256r1DiffieHellman::StaticRng, this) == 0);
        rng = nullptr;
    }

    EcSecP256r1DiffieHellman::~EcSecP256r1DiffieHellman()
    {
        mbedtls_ecp_point_free(&publicKey);
        mbedtls_mpi_free(&privateKey);
    }

    std::array<uint8_t, 65> EcSecP256r1DiffieHellman::PublicKey() const
    {
        return ConvertToBytes<65>(group, publicKey);
    }

    std::array<uint8_t, 32> EcSecP256r1DiffieHellman::SharedSecret(infra::ConstByteRange otherPublicKey, hal::SynchronousRandomDataGenerator& randomDataGenerator) const
    {
        mbedtls_mpi z;
        mbedtls_mpi_init(&z);

        mbedtls_ecp_point dhOtherPublicKey;
        mbedtls_ecp_point_init(&dhOtherPublicKey);
        really_assert(mbedtls_ecp_point_read_binary(&group, &dhOtherPublicKey, otherPublicKey.begin(), otherPublicKey.size()) == 0);

        rng = &randomDataGenerator;
        really_assert(mbedtls_ecdh_compute_shared(const_cast<mbedtls_ecp_group*>(&group), &z, &dhOtherPublicKey, &privateKey, &EcSecP256r1DiffieHellman::StaticRng, const_cast<EcSecP256r1DiffieHellman*>(this)) == 0);
        rng = nullptr;

        auto sharedSecret = ConvertToBytes<32>(z);

        mbedtls_ecp_point_free(&dhOtherPublicKey);
        mbedtls_mpi_free(&z);

        return sharedSecret;
    }

    int EcSecP256r1DiffieHellman::StaticRng(void* self, unsigned char* data, std::size_t size)
    {
        static_cast<EcSecP256r1DiffieHellman*>(self)->rng->GenerateRandomData(infra::MakeRange(data, data + size));
        return 0;
    }

    EcSecP256r1DsaSigner::EcSecP256r1DsaSigner(infra::BoundedConstString dsaCertificatePrivateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
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
        really_assert(mbedtls_pk_parse_key(&dsaPrivateKeyContext, infra::ReinterpretCastByteRange(infra::MakeRange(dsaCertificatePrivateKey)).begin(), dsaCertificatePrivateKey.size(), nullptr, 0, &EcSecP256r1DsaSigner::StaticRng, this) == 0);
        really_assert(mbedtls_ecp_export(mbedtls_pk_ec(dsaPrivateKeyContext), &group, &privateKey, &dsaPublicKey) == 0);
        mbedtls_pk_free(&dsaPrivateKeyContext);
        mbedtls_ecp_point_free(&dsaPublicKey);
    }

    EcSecP256r1DsaSigner::~EcSecP256r1DsaSigner()
    {
        mbedtls_mpi_free(&privateKey);
        mbedtls_ecdh_free(&context);
        mbedtls_ecp_group_free(&group);
    }

    std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> EcSecP256r1DsaSigner::Sign(infra::ConstByteRange data, hal::SynchronousRandomDataGenerator& randomDataGenerator) const
    {
        mbedtls_mpi r;
        mbedtls_mpi_init(&r);
        mbedtls_mpi s;
        mbedtls_mpi_init(&s);

        rng = &randomDataGenerator;
        really_assert(mbedtls_ecdsa_sign(const_cast<mbedtls_ecp_group*>(&group), &r, &s, &privateKey, data.begin(), data.size(), &EcSecP256r1DsaSigner::StaticRng, const_cast<EcSecP256r1DsaSigner*>(this)) == 0);
        rng = nullptr;

        auto encodedR = ConvertToBytes<32>(r);
        auto encodedS = ConvertToBytes<32>(s);

        mbedtls_mpi_free(&s);
        mbedtls_mpi_free(&r);

        return { encodedR, encodedS };
    }

    int EcSecP256r1DsaSigner::StaticRng(void* self, unsigned char* data, std::size_t size)
    {
        static_cast<EcSecP256r1DsaSigner*>(self)->rng->GenerateRandomData(infra::MakeRange(data, data + size));
        return 0;
    }

    EcSecP256r1DsaVerifier::EcSecP256r1DsaVerifier(infra::BoundedConstString dsaCertificate, infra::BoundedConstString rootCaCertificate)
    {
        mbedtls_x509_crt_init(&rootCertificate);
        really_assert(mbedtls_x509_crt_parse(&rootCertificate, infra::ReinterpretCastByteRange(infra::MakeRange(rootCaCertificate)).begin(), rootCaCertificate.size()) == 0);
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

        really_assert(mbedtls_x509_crt_parse(&certificate, infra::ReinterpretCastByteRange(infra::MakeRange(dsaCertificate)).begin(), dsaCertificate.size()) == 0);
        really_assert(mbedtls_pk_get_type(&certificate.pk) == MBEDTLS_PK_ECKEY);

        std::array<uint8_t, 32> hash;
        const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        mbedtls_md(mdInfo, certificate.tbs.p, certificate.tbs.len, hash.data());
        bool verificationOk = mbedtls_pk_verify(&rootCertificate.pk, MBEDTLS_MD_SHA256, hash.data(), hash.size(), certificate.MBEDTLS_PRIVATE(sig).p, certificate.MBEDTLS_PRIVATE(sig).len) == 0;
        really_assert(verificationOk);

        really_assert(mbedtls_ecp_export(mbedtls_pk_ec(certificate.pk), &otherGroup, &otherDsaPrivateKey, &publicKey) == 0);

        mbedtls_x509_crt_free(&certificate);

        mbedtls_mpi_free(&otherDsaPrivateKey);
        mbedtls_ecp_group_free(&otherGroup);
    }

    EcSecP256r1DsaVerifier::~EcSecP256r1DsaVerifier()
    {
        mbedtls_ecp_point_free(&publicKey);
        mbedtls_ecp_group_free(&group);
        mbedtls_x509_crt_free(&rootCertificate);
    }

    bool EcSecP256r1DsaVerifier::Verify(infra::ConstByteRange data, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS) const
    {
        mbedtls_mpi r;
        mbedtls_mpi_init(&r);
        mbedtls_mpi s;
        mbedtls_mpi_init(&s);

        really_assert(mbedtls_mpi_read_binary(&r, signatureR.begin(), signatureR.size()) == 0);
        really_assert(mbedtls_mpi_read_binary(&s, signatureS.begin(), signatureS.size()) == 0);

        bool result = mbedtls_ecdsa_verify(const_cast<mbedtls_ecp_group*>(&group), data.begin(), data.size(), &publicKey, &r, &s) == 0;

        mbedtls_mpi_free(&s);
        mbedtls_mpi_free(&r);

        return result;
    }

    void HmacDrbgSha256::Expand(infra::ConstByteRange seed, infra::ByteRange expandedMaterial) const
    {
        mbedtls_hmac_drbg_context context;
        mbedtls_hmac_drbg_init(&context);

        really_assert(mbedtls_hmac_drbg_seed_buf(&context, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), seed.begin(), seed.size()) == 0);
        really_assert(mbedtls_hmac_drbg_random(&context, expandedMaterial.begin(), expandedMaterial.size()) == 0);

        mbedtls_hmac_drbg_free(&context);
    }

    EchoOnSesameDiffieHellman::EchoOnSesameDiffieHellman(SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString dsaCertificatePrivateKey, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnSesame(secured, serializerFactory, errorPolicy)
        , DiffieHellmanKeyEstablishment(static_cast<services::Echo&>(*this))
        , DiffieHellmanKeyEstablishmentProxy(static_cast<services::Echo&>(*this))
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
        , dsaCertificate(dsaCertificate)
        , rootCaCertificate(rootCaCertificate)
        , signer(dsaCertificatePrivateKey, randomDataGenerator)
    {}

    void EchoOnSesameDiffieHellman::RequestSend(ServiceProxy& serviceProxy)
    {
        if (initializingKeys && &serviceProxy != this)
            waitingProxies.push_back(serviceProxy);
        else
            EchoOnSesame::RequestSend(serviceProxy);
    }

    void EchoOnSesameDiffieHellman::Initialized()
    {
        initializingKeys = true;
        EchoOnSesame::Initialized();

        keyExchange.Emplace(randomDataGenerator);

        DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
            {
                DiffieHellmanKeyEstablishmentProxy::PresentCertificate(dsaCertificate);

                DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
                    {
                        auto encodedDhPublicKey = keyExchange->PublicKey();
                        auto [r, s] = signer.Sign(encodedDhPublicKey, randomDataGenerator);

                        sentExchange = true;
                        DiffieHellmanKeyEstablishmentProxy::Exchange(encodedDhPublicKey, r, s);

                        if (nextKeyPair)
                        {
                            secured.SetNextSendKey(nextKeyPair->first, nextKeyPair->second);
                            nextKeyPair = infra::none;
                        }
                    });
            });
    }

    template<std::size_t Size>
    std::array<uint8_t, Size> Middle(infra::ConstByteRange range, std::size_t start)
    {
        std::array<uint8_t, Size> result;
        infra::Copy(infra::Head(infra::DiscardHead(range, start), Size), infra::MakeRange(result));
        return result;
    }

    void EchoOnSesameDiffieHellman::Exchange(infra::ConstByteRange otherPublicKey, infra::ConstByteRange signatureR, infra::ConstByteRange signatureS)
    {
        if (verifier == infra::none || !verifier->Verify(otherPublicKey, signatureR, signatureS))
            return;

        auto sharedSecret = keyExchange->SharedSecret(otherPublicKey, randomDataGenerator);
        auto publicKey = keyExchange->PublicKey();

        int swap = std::lexicographical_compare(publicKey.begin(), publicKey.end(), otherPublicKey.begin(), otherPublicKey.end()) ? 32 : 0;

        std::array<uint8_t, 64> expandedMaterial{};
        keyExpander.Expand(sharedSecret, expandedMaterial);

        std::array<uint8_t, 16> key = Middle<16>(expandedMaterial, 0 + swap);
        std::array<uint8_t, 16> iv = Middle<16>(expandedMaterial, 16 + swap);
        std::array<uint8_t, 16> otherKey = Middle<16>(expandedMaterial, 32 - swap);
        std::array<uint8_t, 16> otherIv = Middle<16>(expandedMaterial, 48 - swap);

        if (sentExchange)
            secured.SetNextSendKey(key, iv);
        else
            nextKeyPair = { key, iv };

        secured.SetReceiveKey(otherKey, otherIv);

        initializingKeys = false;
        ReQueueWaitingProxies();
        MethodDone();
    }

    void EchoOnSesameDiffieHellman::PresentCertificate(infra::BoundedConstString dsaCertificate)
    {
        verifier.Emplace(dsaCertificate, rootCaCertificate);

        MethodDone();
    }

    void EchoOnSesameDiffieHellman::ReQueueWaitingProxies()
    {
        while (!waitingProxies.empty())
        {
            auto& proxy = waitingProxies.front();
            waitingProxies.pop_front();
            RequestSend(proxy);
        }
    }
}
