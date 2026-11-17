#include "services/util/SesameCryptoMbedTls.hpp"
#include "gmock/gmock.h"
#include <array>
#include <vector>

class AesGcmEncryptionMbedTlsTest
    : public testing::Test
{
public:
    void EncryptInPlace(const std::array<uint8_t, 16>& withKey, const std::array<uint8_t, 12>& withIv, std::vector<uint8_t>& data)
    {
        encryption.EncryptWithKey(infra::MakeRange(withKey));
        encryption.Encrypt(infra::MakeRange(withIv), infra::MakeRange(data), infra::MakeRange(mac));
    }

    bool DecryptInPlace(const std::array<uint8_t, 16>& withKey, const std::array<uint8_t, 12>& withIv, std::vector<uint8_t>& data, const std::array<uint8_t, 16>& withMac)
    {
        decryption.DecryptWithKey(infra::MakeRange(withKey));
        return decryption.Decrypt(infra::MakeRange(withIv), infra::MakeRange(data), infra::MakeRange(withMac));
    }

    const std::array<uint8_t, 16> zeroKey{};
    const std::array<uint8_t, 12> zeroIv{};
    const std::array<uint8_t, 16> key{ 0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08 };
    const std::array<uint8_t, 12> iv{ 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
    const std::vector<uint8_t> plaintext{
        0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
        0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda, 0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
        0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
        0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55
    };
    const std::vector<uint8_t> ciphertext{
        0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
        0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0, 0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e,
        0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
        0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91, 0x47, 0x3f, 0x59, 0x85
    };
    const std::array<uint8_t, 16> expectedMac{ 0x4d, 0x5c, 0x2a, 0xf3, 0x27, 0xcd, 0x64, 0xa6, 0x2c, 0xf3, 0x5a, 0xbd, 0x2b, 0xa6, 0xfa, 0xb4 };

    std::array<uint8_t, 16> mac{};
    services::AesGcmEncryptionMbedTls encryption;
    services::AesGcmEncryptionMbedTls decryption;
};

TEST_F(AesGcmEncryptionMbedTlsTest, encrypt_produces_known_ciphertext_and_mac_in_place)
{
    auto data = plaintext;

    EncryptInPlace(key, iv, data);

    EXPECT_THAT(data, testing::ElementsAreArray(ciphertext));
    EXPECT_THAT(mac, testing::ElementsAreArray(expectedMac));
}

TEST_F(AesGcmEncryptionMbedTlsTest, decrypt_with_correct_mac_returns_true_and_plaintext_in_place)
{
    auto data = ciphertext;

    auto result = DecryptInPlace(key, iv, data, expectedMac);

    EXPECT_THAT(result, testing::IsTrue());
    EXPECT_THAT(data, testing::ElementsAreArray(plaintext));
}

TEST_F(AesGcmEncryptionMbedTlsTest, decrypt_with_tampered_mac_returns_false)
{
    auto data = ciphertext;
    auto tamperedMac = expectedMac;
    tamperedMac[15] ^= 1;

    EXPECT_THAT(DecryptInPlace(key, iv, data, tamperedMac), testing::IsFalse());
}

TEST_F(AesGcmEncryptionMbedTlsTest, decrypt_with_tampered_ciphertext_returns_false)
{
    auto data = ciphertext;
    data[20] ^= 1;

    EXPECT_THAT(DecryptInPlace(key, iv, data, expectedMac), testing::IsFalse());
}

TEST_F(AesGcmEncryptionMbedTlsTest, encrypt_empty_data_produces_known_mac)
{
    std::vector<uint8_t> data;

    EncryptInPlace(zeroKey, zeroIv, data);

    EXPECT_THAT(mac, testing::ElementsAre(0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61, 0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a));
}

TEST_F(AesGcmEncryptionMbedTlsTest, decrypt_empty_data_with_correct_mac_returns_true)
{
    std::vector<uint8_t> data;
    std::array<uint8_t, 16> emptyDataMac{ 0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61, 0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a };

    EXPECT_THAT(DecryptInPlace(zeroKey, zeroIv, data, emptyDataMac), testing::IsTrue());
}

TEST_F(AesGcmEncryptionMbedTlsTest, decrypt_empty_data_with_wrong_mac_returns_false)
{
    std::vector<uint8_t> data;

    EXPECT_THAT(DecryptInPlace(zeroKey, zeroIv, data, std::array<uint8_t, 16>{}), testing::IsFalse());
}

TEST_F(AesGcmEncryptionMbedTlsTest, encrypt_single_zero_block_produces_known_ciphertext)
{
    std::vector<uint8_t> data(16, 0);

    EncryptInPlace(zeroKey, zeroIv, data);

    EXPECT_THAT(data, testing::ElementsAre(0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92, 0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78));
    EXPECT_THAT(mac, testing::ElementsAre(0xab, 0x6e, 0x47, 0xd4, 0x2c, 0xec, 0x13, 0xbd, 0xf5, 0x3a, 0x67, 0xb2, 0x12, 0x57, 0xbd, 0xdf));
}

TEST_F(AesGcmEncryptionMbedTlsTest, non_multiple_of_block_size_round_trips)
{
    std::vector<uint8_t> original(21);
    for (std::size_t i = 0; i != original.size(); ++i)
        original[i] = static_cast<uint8_t>(i);
    auto data = original;

    EncryptInPlace(key, iv, data);
    auto result = DecryptInPlace(key, iv, data, mac);

    EXPECT_THAT(result, testing::IsTrue());
    EXPECT_THAT(data, testing::ElementsAreArray(original));
}
