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

    std::vector<uint8_t> hash = signer.ImageSignature(std::vector<uint8_t>{ 0, 1, 2, 3 });
    EXPECT_EQ(32, hash.size());
    EXPECT_EQ((std::vector<uint8_t>{ 5, 78, 222, 193, 208, 33, 31, 98,
                                     79, 237, 12, 188, 169, 212, 249, 64,
                                     11, 14, 73, 28, 67, 116, 42, 242,
                                     197, 176, 171, 235, 240, 201, 144, 216}), hash);
}
