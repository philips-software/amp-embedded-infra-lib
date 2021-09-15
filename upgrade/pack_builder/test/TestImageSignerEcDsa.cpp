#include "hal/synchronous_interfaces/test_doubles/SynchronousFixedRandomDataGenerator.hpp"
#include "upgrade/pack_builder/ImageSignerEcDsa.hpp"
#include "gtest/gtest.h"

class TestImageSignerEcDsa
    : public testing::Test
{
public:
    TestImageSignerEcDsa()
        : randomDataGenerator(std::vector<uint8_t>(1000, 5))
        , signer(randomDataGenerator, ecDsa224PublicKey, ecDsa224PrivateKey)
    {}

    hal::SynchronousFixedRandomDataGenerator randomDataGenerator;
    application::ImageSignerEcDsa signer;

    const std::array<uint8_t, 56> ecDsa224PublicKey{ { 0x86, 0x9b, 0xa2, 0x7d, 0x39, 0x56, 0x43, 0x16,
        0x1c, 0xfd, 0x34, 0x7f, 0x9e, 0x21, 0x44, 0x88,
        0x21, 0x7e, 0xc5, 0x26, 0xfe, 0x5f, 0x1e, 0xca,
        0x50, 0x6b, 0xce, 0xe2, 0x6f, 0xf6, 0x9d, 0x3b,
        0x1d, 0xcc, 0x98, 0xe9, 0xee, 0x1e, 0xb4, 0xe8,
        0xf4, 0xa5, 0xae, 0x2f, 0x30, 0x6e, 0xab, 0x9b,
        0x7b, 0xb6, 0xbf, 0xe7, 0x3b, 0x16, 0x2a, 0x8e } };

    const std::array<uint8_t, 56> ecDsa224PrivateKey{ { 0xc9, 0x50, 0x62, 0x5a, 0xbe, 0x84, 0x03, 0x8a,
        0xc9, 0x1d, 0x0c, 0x2b, 0x27, 0xc0, 0xc0, 0xe5,
        0xd2, 0xa1, 0xe3, 0xb8, 0x0a, 0x66, 0x0e, 0xde,
        0x99, 0x92, 0xc7, 0x96, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
};

TEST_F(TestImageSignerEcDsa, Sign)
{
    std::vector<uint8_t> signature = signer.ImageSignature(std::vector<uint8_t>{ 0, 1, 2, 3 });
    EXPECT_EQ(56, signature.size());
    EXPECT_EQ((std::vector<uint8_t>{
                  0xbe, 0xe0, 0x71, 0x4a, 0xeb, 0xf1, 0x68, 0x2e,
                  0xfe, 0x16, 0x22, 0xa9, 0x74, 0x66, 0x4e, 0x5b,
                  0x71, 0xe4, 0xdf, 0xfc, 0x3c, 0xdd, 0x24, 0x12,
                  0xb9, 0xa4, 0xb6, 0x2b, 0xdd, 0x7c, 0x7f, 0x96,
                  0x7b, 0x1d, 0xd3, 0xf8, 0x82, 0x8e, 0xc2, 0x15,
                  0x78, 0xa7, 0x73, 0x47, 0xef, 0x66, 0xc9, 0x89,
                  0xe1, 0x6c, 0xcb, 0x22, 0x6d, 0x95, 0x9b, 0x44 }),
        signature);
}
