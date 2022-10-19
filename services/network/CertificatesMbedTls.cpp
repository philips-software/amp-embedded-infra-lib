#include "services/network/CertificatesMbedTls.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "mbedtls/oid.h"
#include "mbedtls/pk.h"
#include "mbedtls/version.h"

#if MBEDTLS_VERSION_MAJOR < 3
#define MBEDTLS_PRIVATE(member) member
#endif

namespace
{
    int RandomDataGeneratorWrapper(void* data, unsigned char* output, std::size_t size)
    {
        reinterpret_cast<hal::SynchronousRandomDataGenerator*>(data)->GenerateRandomData(infra::ByteRange(reinterpret_cast<uint8_t*>(output), reinterpret_cast<uint8_t*>(output) + size));
        return 0;
    }

    infra::ByteRange MakeByteRange(const mbedtls_mpi& number)
    {
        return infra::ByteRange(reinterpret_cast<unsigned char*>(number.MBEDTLS_PRIVATE(p)), reinterpret_cast<unsigned char*>(number.MBEDTLS_PRIVATE(p) + number.MBEDTLS_PRIVATE(n)));
    }

    infra::ConstByteRange MakeConstByteRange(const mbedtls_x509_buf& buffer)
    {
        return infra::ConstByteRange(buffer.p, buffer.p + buffer.len);
    }
}

namespace services
{
    CertificatesMbedTls::CertificatesMbedTls()
    {
        mbedtls_x509_crt_init(&caCertificates);
        mbedtls_x509_crt_init(&ownCertificate);
        mbedtls_pk_init(&privateKey);
    }

    CertificatesMbedTls::~CertificatesMbedTls()
    {
        mbedtls_pk_free(&privateKey);
        mbedtls_x509_crt_free(&caCertificates);
        mbedtls_x509_crt_free(&ownCertificate);
    }

    void CertificatesMbedTls::AddCertificateAuthority(infra::ConstByteRange certificate)
    {
        int result = mbedtls_x509_crt_parse(&caCertificates, reinterpret_cast<const unsigned char*>(certificate.begin()), certificate.size());
        really_assert(result == 0);
    }

    void CertificatesMbedTls::AddCertificateAuthority(const infra::BoundedConstString& certificate)
    {
        int result = mbedtls_x509_crt_parse(&caCertificates, reinterpret_cast<const unsigned char*>(certificate.data()), certificate.size());
        really_assert(result == 0);
    }

    void CertificatesMbedTls::AddOwnCertificate(const infra::BoundedConstString& certificate, const infra::BoundedConstString& key, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        int result = mbedtls_x509_crt_parse(&ownCertificate, reinterpret_cast<const unsigned char*>(certificate.data()), certificate.size());
        really_assert(result == 0);

#if MBEDTLS_VERSION_MAJOR < 3
        result = mbedtls_pk_parse_key(&privateKey, reinterpret_cast<const unsigned char*>(key.data()), key.size(), nullptr, 0);
#else
        result = mbedtls_pk_parse_key(&privateKey, reinterpret_cast<const unsigned char*>(key.data()), key.size(), nullptr, 0, &RandomDataGeneratorWrapper, &randomDataGenerator);
#endif
        really_assert(result == 0);
    }

    void CertificatesMbedTls::AddOwnCertificate(infra::ConstByteRange certificate, infra::ConstByteRange key, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        int result = mbedtls_x509_crt_parse(&ownCertificate, reinterpret_cast<const unsigned char*>(certificate.begin()), certificate.size());
        really_assert(result == 0);

#if MBEDTLS_VERSION_MAJOR < 3
        result = mbedtls_pk_parse_key(&privateKey, reinterpret_cast<const unsigned char*>(key.begin()), key.size(), nullptr, 0);
#else
        result = mbedtls_pk_parse_key(&privateKey, reinterpret_cast<const unsigned char*>(key.begin()), key.size(), nullptr, 0, &RandomDataGeneratorWrapper, &randomDataGenerator);
#endif
        really_assert(result == 0);
    }

    void CertificatesMbedTls::Config(mbedtls_ssl_config& sslConfig)
    {
        mbedtls_ssl_conf_ca_chain(&sslConfig, &caCertificates, nullptr);
        int result = mbedtls_ssl_conf_own_cert(&sslConfig, &ownCertificate, &privateKey);
        really_assert(result == 0);
    }

    void CertificatesMbedTls::GenerateNewKey(hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        if (mbedtls_pk_get_type(&privateKey) != MBEDTLS_PK_RSA)
            return;

        mbedtls_rsa_context* rsaContext = mbedtls_pk_rsa(privateKey);
        really_assert(rsaContext != nullptr);

        size_t keySizeInBits = mbedtls_pk_get_bitlen(&privateKey);
        int32_t exponent = ExtractExponent(*rsaContext);

        int result = mbedtls_rsa_gen_key(rsaContext, &RandomDataGeneratorWrapper, &randomDataGenerator, keySizeInBits, exponent);
        really_assert(result == 0);
    }

    void CertificatesMbedTls::WritePrivateKey(infra::BoundedString& outputBuffer)
    {
        infra::ByteOutputStream::WithStorage<2048> contentsStream;
        infra::Asn1Formatter formatter(contentsStream);
        {
            auto sequence = formatter.StartSequence();

            mbedtls_rsa_context& rsaContext = *mbedtls_pk_rsa(privateKey);

            sequence.Add(uint8_t(rsaContext.MBEDTLS_PRIVATE(ver)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(N)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(E)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(D)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(P)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(Q)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(DP)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(DQ)));
            sequence.AddBigNumber(MakeByteRange(rsaContext.MBEDTLS_PRIVATE(QP)));
        }

        outputBuffer.clear();
        infra::StringOutputStream stream(outputBuffer);
        stream << "-----BEGIN RSA PRIVATE KEY-----\r\n";
        stream << infra::AsBase64(contentsStream.Writer().Processed());
        stream << "-----END RSA PRIVATE KEY-----\r\n";
        stream << '\0';
    }

    void CertificatesMbedTls::WriteOwnCertificate(infra::BoundedString& outputBuffer, hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        infra::ByteOutputStream::WithStorage<2048> contentsStream;

        infra::Asn1Formatter formatter(contentsStream);
        {
            infra::ByteRange tbsBegin;
            infra::ByteRange tbsEnd;

            auto certificateSequence = formatter.StartSequence();
            {
                tbsBegin = contentsStream.Writer().Processed();
                {
                    auto tbsSequence = certificateSequence.StartSequence();

                    // Version
                    {
                        auto versionContext = tbsSequence.StartContextSpecific();
                        versionContext.Add(uint8_t(ownCertificate.version - 1));
                    }

                    // Serial
                    tbsSequence.AddSerial(MakeConstByteRange(ownCertificate.serial));

                    // Signature Object ID
                    X509AddAlgorithm(tbsSequence, ownCertificate.sig_oid);

                    // Issuer Name
                    {
                        auto issuerSequence = tbsSequence.StartSequence();
                        X509AddName(issuerSequence, ownCertificate.issuer);
                    }

                    // Validity
                    {
                        auto timeSequence = tbsSequence.StartSequence();
                        X509AddTime(timeSequence, ownCertificate.valid_from);
                        X509AddTime(timeSequence, ownCertificate.valid_to);
                    }

                    // Subject Name
                    {
                        auto subjectSequence = tbsSequence.StartSequence();
                        X509AddName(subjectSequence, ownCertificate.subject);
                    }

                    // PublicKeyInfo
                    {
                        auto publicKeyInfoSequence = tbsSequence.StartSequence();
                        mbedtls_x509_buf pk_oid;

                        mbedtls_oid_get_oid_by_pk_alg(mbedtls_pk_get_type(&privateKey), const_cast<const char**>(reinterpret_cast<char**>(&pk_oid.p)), &pk_oid.len);

                        X509AddAlgorithm(publicKeyInfoSequence, pk_oid);

                        {
                            auto publicKeyBitString = publicKeyInfoSequence.StartBitString();
                            {
                                auto rsaPublicKeySequence = publicKeyBitString.StartSequence();
                                mbedtls_rsa_context* rsaContext = mbedtls_pk_rsa(privateKey);
                                really_assert(rsaContext != nullptr);

                                rsaPublicKeySequence.AddBigNumber(MakeByteRange(rsaContext->MBEDTLS_PRIVATE(N)));
                                rsaPublicKeySequence.AddBigNumber(MakeByteRange(rsaContext->MBEDTLS_PRIVATE(E)));
                            }
                        }
                    }

                    //  v3 Extensions
                    if (ownCertificate.version == 3)
                        tbsSequence.AddContextSpecific(3, MakeConstByteRange(ownCertificate.v3_ext));
                }

                tbsEnd = contentsStream.Writer().Processed();

                // Sign new certificate
                unsigned char hash[64] = {};

                if (mbedtls_md(mbedtls_md_info_from_type(ownCertificate.MBEDTLS_PRIVATE(sig_md)), tbsBegin.cend(), std::distance(tbsBegin.cend(), tbsEnd.cend()), hash) != 0)
                    std::abort();

                unsigned char signature[MBEDTLS_MPI_MAX_SIZE];
                size_t signatureLength = 0;

#if MBEDTLS_VERSION_MAJOR < 3
                auto result = mbedtls_pk_sign(&privateKey, ownCertificate.sig_md, hash, 0, signature, &signatureLength, &RandomDataGeneratorWrapper, &randomDataGenerator);
#else
                auto result = mbedtls_pk_sign(&privateKey, ownCertificate.MBEDTLS_PRIVATE(sig_md), hash, 0, signature, sizeof(signature), &signatureLength, &RandomDataGeneratorWrapper, &randomDataGenerator);
#endif

                if (result != 0)
                    std::abort();

                X509AddAlgorithm(certificateSequence, ownCertificate.sig_oid);
                certificateSequence.AddBitString(infra::ConstByteRange(signature, signature + signatureLength));
            }
        }

        outputBuffer.clear();
        infra::StringOutputStream stream(outputBuffer);
        stream << "-----BEGIN CERTIFICATE-----\r\n";
        stream << infra::AsBase64(contentsStream.Writer().Processed());
        stream << "-----END CERTIFICATE-----\r\n";
        stream << '\0';
    }

    void CertificatesMbedTls::UpdateValidToDate()
    {
        ownCertificate.valid_to = unlimitedExpirationDate;
    }

    int32_t CertificatesMbedTls::ExtractExponent(const mbedtls_rsa_context& rsaContext) const
    {
        uint32_t exponent = 0;
        mbedtls_mpi_write_binary(&rsaContext.MBEDTLS_PRIVATE(E), reinterpret_cast<unsigned char*>(&exponent), sizeof(uint32_t));
        exponent = (exponent << 16) | (exponent >> 16);
        exponent = ((exponent & 0x00ff00ff) << 8) | ((exponent & 0xff00ff00) >> 8);

        return exponent;
    }

    void CertificatesMbedTls::X509AddAlgorithm(infra::Asn1Formatter& root, const mbedtls_x509_buf& oid) const
    {
        auto algorithmSequence = root.StartSequence();
        algorithmSequence.AddObjectId(MakeConstByteRange(oid));
        algorithmSequence.AddOptional<uint8_t>(infra::none);
    }

    void CertificatesMbedTls::X509AddName(infra::Asn1Formatter& root, const mbedtls_x509_name& name) const
    {
        const mbedtls_x509_name* node = &name;

        while (node)
        {
            auto name = root.StartSet();
            {
                auto attributeTypeAndValueSequence = name.StartSequence();
                attributeTypeAndValueSequence.AddObjectId(MakeConstByteRange(node->oid));
                attributeTypeAndValueSequence.AddPrintableString(MakeConstByteRange(node->val));
            }

            node = node->next;
        }
    }

    void CertificatesMbedTls::X509AddTime(infra::Asn1Formatter& root, const mbedtls_x509_time& time) const
    {
        root.AddTime(time.year, time.mon, time.day, time.hour, time.min, time.sec);
    }
}
