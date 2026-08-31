#include "services/util/SesameCryptoMbedTls.hpp"
#include "services/util/SesameCryptoPsa.hpp"
#include "gmock/gmock.h"
#include <array>
#include <cstdint>
#include <vector>

namespace
{
    struct GcmResult
    {
        std::vector<uint8_t> data;
        std::array<uint8_t, 16> mac{};
    };
}

class SesameCryptoPsaTest
    : public testing::Test
{
public:
    GcmResult Encrypt(services::AesGcmEncryption& encryption, infra::ConstByteRange message)
    {
        encryption.EncryptWithKey(infra::MakeRange(key));
        encryption.Start(infra::MakeRange(iv));

        GcmResult result;
        result.data.resize(message.size());
        auto processed = encryption.Update(message, infra::MakeRange(result.data));
        auto finished = encryption.Finish(infra::DiscardHead(infra::MakeRange(result.data), processed), infra::MakeRange(result.mac));
        result.data.resize(processed + finished);
        return result;
    }

    GcmResult Decrypt(services::AesGcmEncryption& encryption, infra::ConstByteRange ciphertext, std::size_t chunkSize = 0)
    {
        encryption.DecryptWithKey(infra::MakeRange(key));
        encryption.Start(infra::MakeRange(iv));

        if (chunkSize == 0)
            chunkSize = ciphertext.size();

        GcmResult result;
        result.data.resize(ciphertext.size());
        std::size_t processed = 0;
        for (std::size_t offset = 0; offset != ciphertext.size();)
        {
            auto chunk = infra::Head(infra::DiscardHead(ciphertext, offset), chunkSize);
            processed += encryption.Update(chunk, infra::DiscardHead(infra::MakeRange(result.data), processed));
            offset += chunk.size();
        }
        auto finished = encryption.Finish(infra::DiscardHead(infra::MakeRange(result.data), processed), infra::MakeRange(result.mac));
        result.data.resize(processed + finished);
        return result;
    }

    std::vector<uint8_t> plaintext{ 'P', 'S', 'A', ' ', 'G', 'C', 'M', ' ', 'i', 'm', 'p', 'l', 'e', 'm', 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', ' ', 't', 'e', 's', 't' };
    std::array<uint8_t, 16> key{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::array<uint8_t, 12> iv{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };

    services::AesGcmEncryptionPsa psaEncryptor;
    services::AesGcmEncryptionPsa psaDecryptor;
    services::AesGcmEncryptionMbedTls mbedTlsEncryptor;
    services::AesGcmEncryptionMbedTls mbedTlsDecryptor;
};

TEST_F(SesameCryptoPsaTest, encrypt_then_decrypt_recovers_plaintext)
{
    auto encrypted = Encrypt(psaEncryptor, infra::MakeRange(plaintext));
    ASSERT_THAT(encrypted.data, testing::Not(testing::ElementsAreArray(plaintext)));

    auto decrypted = Decrypt(psaDecryptor, infra::MakeRange(encrypted.data));

    EXPECT_THAT(decrypted.data, testing::ElementsAreArray(plaintext));
    EXPECT_THAT(decrypted.mac, testing::ElementsAreArray(encrypted.mac));
}

TEST_F(SesameCryptoPsaTest, psa_encryption_matches_mbedtls_encryption)
{
    auto encryptedByPsa = Encrypt(psaEncryptor, infra::MakeRange(plaintext));
    auto encryptedByMbedTls = Encrypt(mbedTlsEncryptor, infra::MakeRange(plaintext));

    EXPECT_THAT(encryptedByPsa.data, testing::ElementsAreArray(encryptedByMbedTls.data));
    EXPECT_THAT(encryptedByPsa.mac, testing::ElementsAreArray(encryptedByMbedTls.mac));
}

TEST_F(SesameCryptoPsaTest, psa_decryption_recovers_plaintext_encrypted_by_mbedtls)
{
    auto encrypted = Encrypt(mbedTlsEncryptor, infra::MakeRange(plaintext));

    auto decrypted = Decrypt(psaDecryptor, infra::MakeRange(encrypted.data));

    EXPECT_THAT(decrypted.data, testing::ElementsAreArray(plaintext));
    EXPECT_THAT(decrypted.mac, testing::ElementsAreArray(encrypted.mac));
}

TEST_F(SesameCryptoPsaTest, mbedtls_decryption_recovers_plaintext_encrypted_by_psa)
{
    auto encrypted = Encrypt(psaEncryptor, infra::MakeRange(plaintext));

    auto decrypted = Decrypt(mbedTlsDecryptor, infra::MakeRange(encrypted.data));

    EXPECT_THAT(decrypted.data, testing::ElementsAreArray(plaintext));
    EXPECT_THAT(decrypted.mac, testing::ElementsAreArray(encrypted.mac));
}

TEST_F(SesameCryptoPsaTest, decryption_in_single_byte_chunks_recovers_plaintext)
{
    auto encrypted = Encrypt(psaEncryptor, infra::MakeRange(plaintext));

    auto decrypted = Decrypt(psaDecryptor, infra::MakeRange(encrypted.data), 1);

    EXPECT_THAT(decrypted.data, testing::ElementsAreArray(plaintext));
    EXPECT_THAT(decrypted.mac, testing::ElementsAreArray(encrypted.mac));
}

TEST_F(SesameCryptoPsaTest, tampered_ciphertext_produces_different_mac)
{
    auto encrypted = Encrypt(psaEncryptor, infra::MakeRange(plaintext));
    encrypted.data.front() ^= 0xff;

    auto decrypted = Decrypt(psaDecryptor, infra::MakeRange(encrypted.data));

    EXPECT_THAT(decrypted.mac, testing::Not(testing::ElementsAreArray(encrypted.mac)));
}
