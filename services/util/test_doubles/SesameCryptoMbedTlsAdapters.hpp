#ifndef SERVICES_SESAME_CRYPTO_MBED_TLS_ADAPTERS_HPP
#define SERVICES_SESAME_CRYPTO_MBED_TLS_ADAPTERS_HPP

#include "infra/event/EventDispatcher.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "mbedtls/md.h"
#include "mbedtls/x509_crt.h"
#include "services/synchronous_util/SynchronousSesameCryptoMbedTls.hpp"
#include "services/util/SesameCrypto.hpp"
#include <optional>

namespace services
{
    // Software implementations of the asynchronous crypto interfaces that complete through the EventDispatcher
    class AesGcmEncryptionMbedTlsAdapter
        : public AesGcmEncryption
    {
    public:
        void SetEncryptKey(infra::ConstByteRange key) override
        {
            encryption.EncryptWithKey(key);
        }

        void SetDecryptKey(infra::ConstByteRange key) override
        {
            encryption.DecryptWithKey(key);
        }

        void Process(infra::ConstByteRange iv, infra::ByteRange data, infra::ByteRange mac, const infra::Function<void()>& onDone) override
        {
            really_assert(this->onDone == nullptr);

            encryption.Start(iv);
            auto processedSize = encryption.Update(data, data);
            auto moreProcessedSize = encryption.Finish(infra::DiscardHead(data, processedSize), mac);
            really_assert(processedSize + moreProcessedSize == data.size());

            this->onDone = onDone;
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    std::exchange(this->onDone, nullptr)();
                });
        }

    private:
        SynchronousAesGcmEncryptionMbedTls encryption;
        infra::Function<void()> onDone;
    };

    class EcSecP256r1DiffieHellmanMbedTlsAdapter
        : public EcSecP256r1DiffieHellman
    {
    public:
        explicit EcSecP256r1DiffieHellmanMbedTlsAdapter(hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : randomDataGenerator(randomDataGenerator)
        {}

        void GenerateKeyPair(const infra::Function<void(const std::array<uint8_t, 65>& publicKey)>& onDone) override
        {
            onSharedSecret = nullptr;
            keyExchange.emplace(randomDataGenerator);
            publicKey = keyExchange->PublicKey();
            onPublicKey = onDone;
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    if (onPublicKey != nullptr)
                        std::exchange(onPublicKey, nullptr)(publicKey);
                });
        }

        void SharedSecret(infra::ConstByteRange otherPublicKey, const infra::Function<void(const std::array<uint8_t, 32>& sharedSecret)>& onDone) override
        {
            onPublicKey = nullptr;
            sharedSecret = keyExchange->SharedSecret(otherPublicKey);
            onSharedSecret = onDone;
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    if (onSharedSecret != nullptr)
                        std::exchange(onSharedSecret, nullptr)(sharedSecret);
                });
        }

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        std::optional<SynchronousEcSecP256r1DiffieHellmanMbedTls> keyExchange;
        std::array<uint8_t, 65> publicKey{};
        std::array<uint8_t, 32> sharedSecret{};
        infra::Function<void(const std::array<uint8_t, 65>& publicKey)> onPublicKey;
        infra::Function<void(const std::array<uint8_t, 32>& sharedSecret)> onSharedSecret;
    };

    class EcSecP256r1DsaSignerMbedTlsAdapter
        : public EcSecP256r1DsaSigner
    {
    public:
        EcSecP256r1DsaSignerMbedTlsAdapter(infra::ConstByteRange dsaCertificatePrivateKey, hal::SynchronousRandomDataGenerator& randomDataGenerator)
            : signer(dsaCertificatePrivateKey, randomDataGenerator)
        {}

        void Sign(infra::ConstByteRange data, const infra::Function<void(const std::array<uint8_t, 32>& r, const std::array<uint8_t, 32>& s)>& onDone) override
        {
            signature = signer.Sign(data);
            onSigned = onDone;
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    if (onSigned != nullptr)
                        std::exchange(onSigned, nullptr)(signature.first, signature.second);
                });
        }

    private:
        SynchronousEcSecP256r1DsaSignerMbedTls signer;
        std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>> signature{};
        infra::Function<void(const std::array<uint8_t, 32>& r, const std::array<uint8_t, 32>& s)> onSigned;
    };

    class EcSecP256r1DsaVerifierMbedTlsAdapter
        : public EcSecP256r1DsaVerifier
    {
    public:
        void VerifyCertificate(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, const infra::Function<void(bool valid)>& onDone) override
        {
            verifier.emplace(dsaCertificate, rootCaCertificate);
            certificateValid = IsSignedByRoot(dsaCertificate, rootCaCertificate);
            Complete(certificateValid, onDone);
        }

        void Verify(infra::ConstByteRange data, infra::ConstByteRange r, infra::ConstByteRange s, const infra::Function<void(bool valid)>& onDone) override
        {
            Complete(certificateValid && verifier != std::nullopt && verifier->Verify(data, r, s), onDone);
        }

    private:
        static bool IsSignedByRoot(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate)
        {
            mbedtls_x509_crt root;
            mbedtls_x509_crt_init(&root);
            mbedtls_x509_crt certificate;
            mbedtls_x509_crt_init(&certificate);

            really_assert(mbedtls_x509_crt_parse(&root, rootCaCertificate.begin(), rootCaCertificate.size()) == 0);
            really_assert(mbedtls_x509_crt_parse(&certificate, dsaCertificate.begin(), dsaCertificate.size()) == 0);

            std::array<uint8_t, 32> hash;
            mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), certificate.tbs.p, certificate.tbs.len, hash.data());
            bool valid = mbedtls_pk_verify(&root.pk, MBEDTLS_MD_SHA256, hash.data(), hash.size(), certificate.MBEDTLS_PRIVATE(sig).p, certificate.MBEDTLS_PRIVATE(sig).len) == 0;

            mbedtls_x509_crt_free(&certificate);
            mbedtls_x509_crt_free(&root);
            return valid;
        }

        void Complete(bool valid, const infra::Function<void(bool valid)>& onDone)
        {
            result = valid;
            onVerified = onDone;
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    if (onVerified != nullptr)
                        std::exchange(onVerified, nullptr)(result);
                });
        }

        std::optional<SynchronousEcSecP256r1DsaVerifierMbedTls> verifier;
        bool certificateValid = false;
        bool result = false;
        infra::Function<void(bool valid)> onVerified;
    };
}

#endif
