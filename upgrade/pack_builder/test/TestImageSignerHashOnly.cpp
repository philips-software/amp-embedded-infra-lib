#include "gtest/gtest.h"
#include "upgrade/pack_builder/ImageSignerHashOnly.hpp"

TEST(TestImageSignerHashOnly, should_report_algorithm_details)
{
    application::ImageSignerHashOnly signer;

    EXPECT_EQ(0, signer.SignatureMethod());
    EXPECT_EQ(32, signer.SignatureLength());
}

TEST(TestImageSignerHashOnly, should_hash_image)
{
    application::ImageSignerHashOnly signer;

    auto hash = signer.ImageSignature(std::vector<uint8_t>{ 0, 1, 2, 3 });
    EXPECT_EQ(32, hash.size());
    EXPECT_EQ((std::vector<uint8_t>{ 0x05, 0x4E, 0xDE, 0xC1, 0xD0, 0x21, 0x1F, 0x62,
                                     0x4F, 0xED, 0x0C, 0xBC, 0xA9, 0xD4, 0xF9, 0x40,
                                     0x0B, 0x0E, 0x49, 0x1C, 0x43, 0x74, 0x2A, 0xF2,
                                     0xC5, 0xB0, 0xAB, 0xEB, 0xF0, 0xC9, 0x90, 0xD8 }), hash);
}

TEST(TestImageSignerHashOnly, should_check_signature)
{
    application::ImageSignerHashOnly signer;
    auto image = std::vector<uint8_t>{ 0, 1, 2, 3 };
    auto hash = signer.ImageSignature(image);

    EXPECT_TRUE(signer.CheckSignature(hash, image));
}
