#ifndef SERVICES_SESAME_CRYPTO_PSA_HPP
#define SERVICES_SESAME_CRYPTO_PSA_HPP

#include "psa/crypto.h"
#include "services/util/SesameCrypto.hpp"

namespace services
{
    class AesGcmEncryptionPsa
        : public AesGcmEncryption
    {
    public:
        AesGcmEncryptionPsa();
        AesGcmEncryptionPsa(const AesGcmEncryptionPsa& other) = delete;
        AesGcmEncryptionPsa& operator=(const AesGcmEncryptionPsa& other) = delete;
        ~AesGcmEncryptionPsa();

        void EncryptWithKey(infra::ConstByteRange key) override;
        void DecryptWithKey(infra::ConstByteRange key) override;
        void Start(infra::ConstByteRange iv) override;
        std::size_t Update(infra::ConstByteRange from, infra::ByteRange to) override;
        std::size_t Finish(infra::ByteRange to, infra::ByteRange mac) override;

    private:
        void SetKey(infra::ConstByteRange key);
        void UpdateTag(infra::ConstByteRange plaintext);

    private:
        bool encrypt = false;
        psa_key_id_t keyId = 0;
        psa_aead_operation_t operation = psa_aead_operation_init();
        psa_aead_operation_t tagOperation = psa_aead_operation_init();
    };
}

#endif
