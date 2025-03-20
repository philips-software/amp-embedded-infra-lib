#include "services/util/EchoOnSesameDiffieHellman.hpp"
#include "mbedtls/ecdsa.h"
#include "mbedtls/hmac_drbg.h"

namespace services
{
    namespace
    {
        template<std::size_t Size>
        std::array<uint8_t, Size> Convert(infra::ConstByteRange value)
        {
            really_assert(Size == value.size());
            std::array<uint8_t, Size> result;
            infra::Copy(value, infra::MakeRange(result));
            return result;
        }
    }

    EchoOnSesameDiffieHellman::EchoOnSesameDiffieHellman(SesameSecured& secured, infra::BoundedConstString dsaCertificate, infra::BoundedConstString dsaCertificatePrivateKey, infra::BoundedConstString rootCaCertificate, hal::SynchronousRandomDataGenerator& randomDataGenerator, MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : EchoOnSesame(secured, serializerFactory, errorPolicy)
        , DiffieHellmanKeyEstablishment(static_cast<services::Echo&>(*this))
        , DiffieHellmanKeyEstablishmentProxy(static_cast<services::Echo&>(*this))
        , secured(secured)
        , randomDataGenerator(randomDataGenerator)
        , dsaCertificate(dsaCertificate)
    {
        mbedtls_x509_crt_init(&rootCertificate);
        really_assert(mbedtls_x509_crt_parse(&rootCertificate, infra::ReinterpretCastByteRange(infra::MakeRange(rootCaCertificate)).begin(), rootCaCertificate.size()) == 0);
        really_assert(mbedtls_pk_get_type(&rootCertificate.pk) == MBEDTLS_PK_ECKEY);

        mbedtls_ecp_group_init(&group);
        really_assert(mbedtls_ecp_group_load(&group, MBEDTLS_ECP_DP_SECP256R1) == 0);

        mbedtls_mpi_init(&dhPrivateKey);
        mbedtls_ecp_point_init(&dhPublicKey);

        mbedtls_ecdh_init(&ecdhContext);
        really_assert(mbedtls_ecdh_setup(&ecdhContext, group.id) == 0);

        mbedtls_mpi_init(&dsaPrivateKey);
        mbedtls_ecp_point dsaPublicKey;
        mbedtls_ecp_point_init(&dsaPublicKey);
        mbedtls_ecp_point_init(&otherDsaPublicKey);

        mbedtls_pk_context dsaPrivateKeyContext;
        mbedtls_pk_init(&dsaPrivateKeyContext);
        really_assert(mbedtls_pk_parse_key(&dsaPrivateKeyContext, infra::ReinterpretCastByteRange(infra::MakeRange(dsaCertificatePrivateKey)).begin(), dsaCertificatePrivateKey.size(), nullptr, 0, &EchoOnSesameDiffieHellman::StaticRng, this) == 0);
        really_assert(mbedtls_ecp_export(mbedtls_pk_ec(dsaPrivateKeyContext), &group, &dsaPrivateKey, &dsaPublicKey) == 0);
        mbedtls_pk_free(&dsaPrivateKeyContext);
        mbedtls_ecp_point_free(&dsaPublicKey);
    }

    EchoOnSesameDiffieHellman::~EchoOnSesameDiffieHellman()
    {
        mbedtls_ecp_point_free(&otherDsaPublicKey);
        mbedtls_mpi_free(&dsaPrivateKey);
        mbedtls_ecdh_free(&ecdhContext);
        mbedtls_ecp_point_free(&dhPublicKey);
        mbedtls_mpi_free(&dhPrivateKey);
        mbedtls_ecp_group_free(&group);
        mbedtls_x509_crt_free(&rootCertificate);
    }

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

        really_assert(mbedtls_ecdh_gen_public(&group, &dhPrivateKey, &dhPublicKey, &EchoOnSesameDiffieHellman::StaticRng, this) == 0);

        DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
            {
                DiffieHellmanKeyEstablishmentProxy::PresentCertificate(dsaCertificate);

                DiffieHellmanKeyEstablishmentProxy::RequestSend([this]()
                    {
                        std::array<uint8_t, 65> encodedDhPublicKey;
                        std::size_t size = 0;
                        really_assert(mbedtls_ecp_point_write_binary(&group, &dhPublicKey, MBEDTLS_ECP_PF_UNCOMPRESSED, &size, encodedDhPublicKey.data(), encodedDhPublicKey.size()) == 0);
                        really_assert(size == encodedDhPublicKey.size());

                        mbedtls_mpi r;
                        mbedtls_mpi_init(&r);
                        mbedtls_mpi s;
                        mbedtls_mpi_init(&s);
                        really_assert(mbedtls_ecdsa_sign(&group, &r, &s, &dsaPrivateKey, encodedDhPublicKey.data(), encodedDhPublicKey.size(), &EchoOnSesameDiffieHellman::StaticRng, this) == 0);

                        std::array<uint8_t, 32> encodedR;
                        really_assert(mbedtls_mpi_write_binary(&r, encodedR.data(), encodedR.size()) == 0);
                        std::array<uint8_t, 32> encodedS;
                        really_assert(mbedtls_mpi_write_binary(&s, encodedS.data(), encodedS.size()) == 0);

                        mbedtls_mpi_free(&s);
                        mbedtls_mpi_free(&r);

                        sentExchange = true;
                        DiffieHellmanKeyEstablishmentProxy::Exchange(encodedDhPublicKey, encodedR, encodedS);

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
        mbedtls_mpi r;
        mbedtls_mpi_init(&r);
        mbedtls_mpi s;
        mbedtls_mpi_init(&s);

        really_assert(mbedtls_mpi_read_binary(&r, signatureR.begin(), signatureR.size()) == 0);
        really_assert(mbedtls_mpi_read_binary(&s, signatureS.begin(), signatureS.size()) == 0);

        really_assert(mbedtls_ecdsa_verify(&group, otherPublicKey.begin(), otherPublicKey.size(), &otherDsaPublicKey, &r, &s) == 0);

        mbedtls_mpi_free(&s);
        mbedtls_mpi_free(&r);

        mbedtls_mpi z;
        mbedtls_mpi_init(&z);

        mbedtls_ecp_point dhOtherPublicKey;
        mbedtls_ecp_point_init(&dhOtherPublicKey);
        // todo; handle error
        really_assert(mbedtls_ecp_point_read_binary(&group, &dhOtherPublicKey, otherPublicKey.begin(), otherPublicKey.size()) == 0);

        really_assert(mbedtls_ecdh_compute_shared(&group, &z, &dhOtherPublicKey, &dhPrivateKey, &EchoOnSesameDiffieHellman::StaticRng, this) == 0);

        std::array<uint8_t, 32> sharedSecret;
        really_assert(mbedtls_mpi_write_binary(&z, sharedSecret.data(), sharedSecret.size()) == 0);

        std::array<uint8_t, 65> publicKey;
        std::size_t size = 0;
        really_assert(mbedtls_ecp_point_write_binary(&group, &dhPublicKey, MBEDTLS_ECP_PF_UNCOMPRESSED, &size, publicKey.data(), publicKey.size()) == 0);
        really_assert(size == publicKey.size());

        int swap = std::lexicographical_compare(publicKey.begin(), publicKey.end(), otherPublicKey.begin(), otherPublicKey.end()) ? 32 : 0;

        mbedtls_ecp_point_free(&dhOtherPublicKey);
        mbedtls_mpi_free(&z);

        std::array<uint8_t, 64> expandedMaterial{};
        ExpandMaterial(sharedSecret, expandedMaterial);

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

        really_assert(mbedtls_ecp_export(mbedtls_pk_ec(certificate.pk), &otherGroup, &otherDsaPrivateKey, &otherDsaPublicKey) == 0);

        mbedtls_x509_crt_free(&certificate);

        mbedtls_mpi_free(&otherDsaPrivateKey);
        mbedtls_ecp_group_free(&otherGroup);

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

    void EchoOnSesameDiffieHellman::ExpandMaterial(infra::ConstByteRange sharedSecret, infra::ByteRange expandedMaterial) const
    {
        mbedtls_hmac_drbg_context context;
        mbedtls_hmac_drbg_init(&context);

        really_assert(mbedtls_hmac_drbg_seed_buf(&context, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), sharedSecret.begin(), sharedSecret.size()) == 0);
        really_assert(mbedtls_hmac_drbg_random(&context, expandedMaterial.begin(), expandedMaterial.size()) == 0);

        mbedtls_hmac_drbg_free(&context);
    }

    int EchoOnSesameDiffieHellman::StaticRng(void* self, unsigned char* data, std::size_t size)
    {
        static_cast<EchoOnSesameDiffieHellman*>(self)->randomDataGenerator.GenerateRandomData(infra::MakeRange(data, data + size));
        return 0;
    }
}
