#include "services/util/SesameCryptoPsa.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <algorithm>
#include <array>

namespace services
{
    AesGcmEncryptionPsa::AesGcmEncryptionPsa()
    {
        really_assert(psa_crypto_init() == PSA_SUCCESS);
    }

    AesGcmEncryptionPsa::~AesGcmEncryptionPsa()
    {
        psa_aead_abort(&operation);
        psa_aead_abort(&tagOperation);

        if (keyId != 0)
            psa_destroy_key(keyId);
    }

    void AesGcmEncryptionPsa::EncryptWithKey(infra::ConstByteRange key)
    {
        encrypt = true;
        SetKey(key);
    }

    void AesGcmEncryptionPsa::DecryptWithKey(infra::ConstByteRange key)
    {
        encrypt = false;
        SetKey(key);
    }

    void AesGcmEncryptionPsa::Start(infra::ConstByteRange iv)
    {
        psa_aead_abort(&operation);
        psa_aead_abort(&tagOperation);

        if (encrypt)
            really_assert(psa_aead_encrypt_setup(&operation, keyId, PSA_ALG_GCM) == PSA_SUCCESS);
        else
            really_assert(psa_aead_decrypt_setup(&operation, keyId, PSA_ALG_GCM) == PSA_SUCCESS);

        really_assert(psa_aead_set_nonce(&operation, iv.begin(), iv.size()) == PSA_SUCCESS);

        if (!encrypt)
        {
            really_assert(psa_aead_encrypt_setup(&tagOperation, keyId, PSA_ALG_GCM) == PSA_SUCCESS);
            really_assert(psa_aead_set_nonce(&tagOperation, iv.begin(), iv.size()) == PSA_SUCCESS);
        }
    }

    std::size_t AesGcmEncryptionPsa::Update(infra::ConstByteRange from, infra::ByteRange to)
    {
        std::size_t processedSize = 0;
        really_assert(psa_aead_update(&operation, from.begin(), from.size(), to.begin(), to.size(), &processedSize) == PSA_SUCCESS);

        if (!encrypt)
            UpdateTag(infra::Head(to, processedSize));

        return processedSize;
    }

    std::size_t AesGcmEncryptionPsa::Finish(infra::ByteRange to, infra::ByteRange mac)
    {
        std::size_t processedSize = 0;
        std::size_t macSize = 0;

        if (!encrypt)
            psa_aead_abort(&operation);

        auto& finishOperation = encrypt ? operation : tagOperation;
        really_assert(psa_aead_finish(&finishOperation, to.begin(), to.size(), &processedSize, mac.begin(), mac.size(), &macSize) == PSA_SUCCESS);

        return processedSize;
    }

    void AesGcmEncryptionPsa::SetKey(infra::ConstByteRange key)
    {
        if (keyId != 0)
        {
            psa_destroy_key(keyId);
            keyId = 0;
        }

        auto attributes = psa_key_attributes_init();
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
        psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
        psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
        really_assert(psa_import_key(&attributes, key.begin(), key.size(), &keyId) == PSA_SUCCESS);
        psa_reset_key_attributes(&attributes);
    }

    void AesGcmEncryptionPsa::UpdateTag(infra::ConstByteRange plaintext)
    {
        std::array<uint8_t, 64> discard;

        for (std::size_t offset = 0; offset != plaintext.size();)
        {
            auto chunkSize = std::min(plaintext.size() - offset, discard.size());
            std::size_t produced = 0;
            really_assert(psa_aead_update(&tagOperation, plaintext.begin() + offset, chunkSize, discard.data(), discard.size(), &produced) == PSA_SUCCESS);
            offset += chunkSize;
        }
    }
}
