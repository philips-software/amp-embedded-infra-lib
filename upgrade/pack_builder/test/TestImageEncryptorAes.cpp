#include "gtest/gtest.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousFixedRandomDataGenerator.hpp"
#include "upgrade/pack_builder/ImageEncryptorAes.hpp"

class ImageEncryptorAesTest
    : public testing::Test
{
public:
    ImageEncryptorAesTest()
        : randomDataGenerator(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 })
        , encryptor(randomDataGenerator, aesKey)
    {}

    hal::SynchronousFixedRandomDataGenerator randomDataGenerator;
    application::ImageEncryptorAes encryptor;

    const std::array<uint8_t, 16> aesKey { {
        0x50, 0x8b, 0xa9, 0xd4, 0x81, 0x50, 0x64, 0xcb,
        0xf2, 0x9b, 0xde, 0xf3, 0x7f, 0x6a, 0x54, 0x3e
    } };
};

TEST_F(ImageEncryptorAesTest, Encrypt)
{
    EXPECT_EQ((std::vector<uint8_t>{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x22, 0x40, 0xf8, 0x03 }), encryptor.Secure(std::vector<uint8_t>{1, 2, 3, 4}));
}
