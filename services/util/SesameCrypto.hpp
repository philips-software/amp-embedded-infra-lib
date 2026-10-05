#ifndef SERVICES_SESAME_CRYPTO_HPP
#define SERVICES_SESAME_CRYPTO_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include <array>
#include <cstdint>

namespace services
{
    // Input ranges only need to stay valid during the call, unless stated otherwise.
    // Starting an operation while a previous one on the same object is pending cancels the previous one; its onDone is not called.
    class EcSecP256r1DiffieHellman
    {
    public:
        EcSecP256r1DiffieHellman() = default;
        EcSecP256r1DiffieHellman(const EcSecP256r1DiffieHellman& other) = delete;
        EcSecP256r1DiffieHellman& operator=(const EcSecP256r1DiffieHellman& other) = delete;

        virtual void GenerateKeyPair(const infra::Function<void(const std::array<uint8_t, 65>& publicKey)>& onDone) = 0;
        virtual void SharedSecret(infra::ConstByteRange otherPublicKey, const infra::Function<void(const std::array<uint8_t, 32>& sharedSecret)>& onDone) = 0;
    };

    class EcSecP256r1DsaSigner
    {
    public:
        EcSecP256r1DsaSigner() = default;
        EcSecP256r1DsaSigner(const EcSecP256r1DsaSigner& other) = delete;
        EcSecP256r1DsaSigner& operator=(const EcSecP256r1DsaSigner& other) = delete;

        virtual void Sign(infra::ConstByteRange data, const infra::Function<void(const std::array<uint8_t, 32>& r, const std::array<uint8_t, 32>& s)>& onDone) = 0;
    };

    class EcSecP256r1DsaVerifier
    {
    public:
        EcSecP256r1DsaVerifier() = default;
        EcSecP256r1DsaVerifier(const EcSecP256r1DsaVerifier& other) = delete;
        EcSecP256r1DsaVerifier& operator=(const EcSecP256r1DsaVerifier& other) = delete;

        virtual void VerifyCertificate(infra::ConstByteRange dsaCertificate, infra::ConstByteRange rootCaCertificate, const infra::Function<void(bool valid)>& onDone) = 0;
        // Uses the public key of the last certificate passed to VerifyCertificate; fails when that certificate was invalid
        virtual void Verify(infra::ConstByteRange data, infra::ConstByteRange r, infra::ConstByteRange s, const infra::Function<void(bool valid)>& onDone) = 0;
    };

    class AesGcmEncryption
    {
    public:
        AesGcmEncryption() = default;
        AesGcmEncryption(const AesGcmEncryption& other) = delete;
        AesGcmEncryption& operator=(const AesGcmEncryption& other) = delete;

        virtual void SetEncryptKey(infra::ConstByteRange key) = 0;
        virtual void SetDecryptKey(infra::ConstByteRange key) = 0;
        // Transforms data in place and writes the computed tag to mac; data and mac must stay valid until onDone.
        // Only one Process may be pending at a time
        virtual void Process(infra::ConstByteRange iv, infra::ByteRange data, infra::ByteRange mac, const infra::Function<void()>& onDone) = 0;
    };
}

#endif
