#include "hal/synchronous_interfaces/test_doubles/SynchronousFixedRandomDataGenerator.hpp"
#include "upgrade/pack_builder/ImageSignerRsa.hpp"
#include "gtest/gtest.h"

class TestImageSignerRsa
    : public testing::Test
{
public:
    TestImageSignerRsa()
        : randomDataGenerator(std::vector<uint8_t>(1000, 5))
        , rsaPublicKey(N, E)
        , rsaPrivateKey(D, P, Q)
        , signer(randomDataGenerator, rsaPublicKey, rsaPrivateKey)
    {}

    hal::SynchronousFixedRandomDataGenerator randomDataGenerator;
    RsaPublicKey rsaPublicKey;
    RsaPrivateKey rsaPrivateKey;
    application::ImageSignerRsa signer;

    const std::array<uint8_t, 128> N{ {
        0x6f,
        0x62,
        0xf5,
        0xf1,
        0x58,
        0x0b,
        0xb7,
        0x03,
        0xc6,
        0x71,
        0x68,
        0x87,
        0x74,
        0xe3,
        0xda,
        0x36,
        0x5f,
        0xbf,
        0x43,
        0x0e,
        0xc3,
        0xad,
        0xba,
        0x9e,
        0xf2,
        0x34,
        0xb9,
        0x10,
        0x9d,
        0x0a,
        0x3d,
        0x10,
        0xf7,
        0xb6,
        0xb5,
        0x71,
        0x27,
        0x10,
        0xf3,
        0x53,
        0x49,
        0x0d,
        0x1a,
        0x19,
        0x92,
        0xf4,
        0x33,
        0xac,
        0x4a,
        0xe6,
        0xe8,
        0x11,
        0x78,
        0x42,
        0x38,
        0xf6,
        0x81,
        0xd1,
        0xfd,
        0x9f,
        0x7a,
        0x9f,
        0xf5,
        0x4f,
        0x61,
        0xd5,
        0x6a,
        0xf3,
        0x5e,
        0x60,
        0xe8,
        0xba,
        0x10,
        0xf0,
        0x79,
        0xb1,
        0xc1,
        0x0f,
        0xef,
        0x65,
        0x39,
        0x00,
        0xbd,
        0xd7,
        0xec,
        0x3a,
        0x73,
        0xec,
        0xb7,
        0x3f,
        0x6e,
        0xba,
        0xc6,
        0x12,
        0x37,
        0x33,
        0x2d,
        0xa8,
        0xbf,
        0xe2,
        0xf6,
        0x50,
        0x26,
        0xae,
        0xfd,
        0xbd,
        0x98,
        0xe3,
        0x87,
        0x36,
        0xa4,
        0x63,
        0x6d,
        0x71,
        0xd8,
        0x22,
        0xe1,
        0xdb,
        0x6f,
        0x02,
        0xe6,
        0x8c,
        0x69,
        0xb8,
        0x65,
        0x5c,
        0x98,
        0xc6,
    } };
    const std::array<uint8_t, 4> E{ { 0x01, 0x00, 0x01, 0x00 } };
    const std::array<uint8_t, 128> D{ {
        0xb1,
        0xc7,
        0x3e,
        0xa6,
        0xa8,
        0x82,
        0x2a,
        0xec,
        0x58,
        0xe0,
        0x0e,
        0x1e,
        0xd6,
        0xdc,
        0xea,
        0xe9,
        0x5b,
        0x0e,
        0xf6,
        0x81,
        0xb2,
        0x53,
        0x19,
        0x6a,
        0x2f,
        0xa8,
        0xdb,
        0xf4,
        0x5f,
        0xab,
        0x8c,
        0x02,
        0x50,
        0x32,
        0x0a,
        0x26,
        0xa3,
        0xf7,
        0x1f,
        0xb2,
        0xd4,
        0x84,
        0xf4,
        0x3a,
        0x3a,
        0x78,
        0x74,
        0xeb,
        0x89,
        0xd3,
        0xdf,
        0x20,
        0xef,
        0xbe,
        0xa5,
        0xf7,
        0x88,
        0x8e,
        0xac,
        0xd1,
        0x9c,
        0x02,
        0x9e,
        0x93,
        0x2a,
        0x3c,
        0xd7,
        0xb7,
        0xa0,
        0x17,
        0xcc,
        0x3e,
        0x26,
        0x6d,
        0x80,
        0xc7,
        0xe6,
        0xd1,
        0xe4,
        0xb3,
        0x2e,
        0x67,
        0xde,
        0xc7,
        0x94,
        0x16,
        0x15,
        0xa6,
        0xce,
        0x51,
        0xde,
        0x78,
        0xc6,
        0xb7,
        0x9c,
        0x71,
        0x31,
        0xf5,
        0x1c,
        0x34,
        0xae,
        0xc5,
        0x9e,
        0x19,
        0x49,
        0x3a,
        0x08,
        0xab,
        0x04,
        0x8d,
        0xe4,
        0x0f,
        0xb0,
        0xaf,
        0x8e,
        0x7f,
        0xd9,
        0xda,
        0x48,
        0xe3,
        0x8c,
        0x6d,
        0x0f,
        0x5c,
        0x9b,
        0xcb,
        0x52,
        0x5a,
    } };
    const std::array<uint8_t, 64> P{ {
        0xb9,
        0x7f,
        0x15,
        0xa4,
        0x95,
        0xf7,
        0x1f,
        0x8e,
        0x6c,
        0xf9,
        0x37,
        0xd9,
        0x44,
        0x36,
        0x36,
        0xe6,
        0x98,
        0x9f,
        0x59,
        0xed,
        0xb3,
        0xb8,
        0x26,
        0x09,
        0x2f,
        0x90,
        0x7b,
        0xac,
        0xf4,
        0xff,
        0x49,
        0x9c,
        0x42,
        0xaa,
        0x1f,
        0x94,
        0xb5,
        0xba,
        0xf5,
        0x81,
        0xdd,
        0xb9,
        0x09,
        0x5b,
        0x71,
        0xbf,
        0x15,
        0x4e,
        0x49,
        0x04,
        0x57,
        0x55,
        0x82,
        0x1e,
        0x36,
        0xfd,
        0x46,
        0x05,
        0x25,
        0x10,
        0x1a,
        0xc8,
        0x6c,
        0xf2,
    } };
    const std::array<uint8_t, 64> Q{ {
        0x67,
        0x77,
        0x70,
        0x7e,
        0xe1,
        0xb7,
        0x8f,
        0x2b,
        0xb1,
        0x99,
        0xa2,
        0xa5,
        0xa0,
        0x49,
        0x0d,
        0x7c,
        0xfb,
        0x56,
        0x78,
        0xf0,
        0xae,
        0xaa,
        0x0b,
        0xc6,
        0xd7,
        0x55,
        0x03,
        0xfc,
        0x3b,
        0x3c,
        0x5c,
        0x9a,
        0x39,
        0x91,
        0x36,
        0x98,
        0x91,
        0x21,
        0xf8,
        0x5d,
        0x9b,
        0x3a,
        0x90,
        0xea,
        0xcc,
        0x17,
        0xf6,
        0xb2,
        0xc6,
        0xf1,
        0xc7,
        0x17,
        0x11,
        0xe5,
        0x5b,
        0x72,
        0x9b,
        0x43,
        0xca,
        0x13,
        0xed,
        0x44,
        0xb7,
        0xd1,
    } };
};

TEST_F(TestImageSignerRsa, Sign)
{
    std::vector<uint8_t> signature = signer.ImageSignature(std::vector<uint8_t>{ 0, 1, 2, 3 });
    EXPECT_EQ(128, signature.size());
    EXPECT_EQ((std::vector<uint8_t>{
                  0x93, 0xd3, 0x6d, 0xa4, 0xa5, 0x25, 0xd0, 0x7f,
                  0xd0, 0x1d, 0x6e, 0x91, 0x15, 0x0d, 0x4f, 0xca,
                  0xaf, 0x4e, 0xb0, 0xa5, 0x5c, 0x7f, 0xa2, 0xc1,
                  0xa5, 0x30, 0xba, 0xe9, 0x60, 0xbf, 0x3c, 0x1e,
                  0x30, 0x9c, 0xe2, 0x0b, 0x6e, 0x49, 0xa5, 0x11,
                  0xfb, 0x31, 0xd6, 0x42, 0x65, 0x4d, 0x44, 0x9a,
                  0x35, 0x51, 0xe9, 0xa4, 0xe7, 0xd1, 0x03, 0x2b,
                  0x80, 0x5b, 0x25, 0x58, 0x85, 0x15, 0xd7, 0x40,
                  0x43, 0xb2, 0xa7, 0x52, 0xcc, 0x7a, 0xf7, 0xa3,
                  0x98, 0x00, 0x3d, 0x1d, 0xa5, 0xca, 0xca, 0x7b,
                  0x29, 0x37, 0x2d, 0x4b, 0x66, 0xb2, 0x65, 0x5b,
                  0x41, 0x34, 0x12, 0x0e, 0x15, 0xfe, 0xaf, 0xbe,
                  0x03, 0x41, 0xb5, 0x6f, 0x82, 0x06, 0x78, 0x95,
                  0x6b, 0x18, 0x7a, 0x35, 0x17, 0x96, 0x90, 0x75,
                  0x8d, 0x78, 0x8f, 0x06, 0x6c, 0x72, 0xb6, 0x08,
                  0x35, 0x15, 0xa2, 0x80, 0xaa, 0xd0, 0xe4, 0xe8 }),
        signature);
}

TEST_F(TestImageSignerRsa, CheckSignature)
{
    std::vector<uint8_t> signature{ { 0x93, 0xd3, 0x6d, 0xa4, 0xa5, 0x25, 0xd0, 0x7f,
        0xd0, 0x1d, 0x6e, 0x91, 0x15, 0x0d, 0x4f, 0xca,
        0xaf, 0x4e, 0xb0, 0xa5, 0x5c, 0x7f, 0xa2, 0xc1,
        0xa5, 0x30, 0xba, 0xe9, 0x60, 0xbf, 0x3c, 0x1e,
        0x30, 0x9c, 0xe2, 0x0b, 0x6e, 0x49, 0xa5, 0x11,
        0xfb, 0x31, 0xd6, 0x42, 0x65, 0x4d, 0x44, 0x9a,
        0x35, 0x51, 0xe9, 0xa4, 0xe7, 0xd1, 0x03, 0x2b,
        0x80, 0x5b, 0x25, 0x58, 0x85, 0x15, 0xd7, 0x40,
        0x43, 0xb2, 0xa7, 0x52, 0xcc, 0x7a, 0xf7, 0xa3,
        0x98, 0x00, 0x3d, 0x1d, 0xa5, 0xca, 0xca, 0x7b,
        0x29, 0x37, 0x2d, 0x4b, 0x66, 0xb2, 0x65, 0x5b,
        0x41, 0x34, 0x12, 0x0e, 0x15, 0xfe, 0xaf, 0xbe,
        0x03, 0x41, 0xb5, 0x6f, 0x82, 0x06, 0x78, 0x95,
        0x6b, 0x18, 0x7a, 0x35, 0x17, 0x96, 0x90, 0x75,
        0x8d, 0x78, 0x8f, 0x06, 0x6c, 0x72, 0xb6, 0x08,
        0x35, 0x15, 0xa2, 0x80, 0xaa, 0xd0, 0xe4, 0xe8 } };

    EXPECT_TRUE(signer.CheckSignature(signature, std::vector<uint8_t>{ 0, 1, 2, 3 }));
}
