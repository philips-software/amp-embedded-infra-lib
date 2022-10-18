#include "services/util/FlashAlign.hpp"
#include "gtest/gtest.h"

class FlashAlign4Bytes : public testing::Test
{
public:
    services::FlashAlign::WithAlignment<4> align;
};

class FlashAlign16Bytes : public testing::Test
{
public:
    services::FlashAlign::WithAlignment<16> align;
};

TEST_F(FlashAlign4Bytes, Align_0_bytes)
{
    std::array<uint8_t, 0> data = {};
    align.Align(0, data);

    EXPECT_EQ(nullptr, align.First());
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign16Bytes, Align_0_bytes)
{
    std::array<uint8_t, 0> data = {};
    align.Align(0, data);

    EXPECT_EQ(nullptr, align.First());
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign4Bytes, Align_single_byte_aligned_address)
{
    std::array<uint8_t, 1> data = { 1 };
    align.Align(0, data);

    EXPECT_EQ(0, align.First()->alignedAddress);
    EXPECT_EQ((std::array<int, 4>{ 1, 0xff, 0xff, 0xff }), align.First()->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign16Bytes, Align_single_byte_aligned_address)
{
    std::array<uint8_t, 1> data = { 1 };
    align.Align(0, data);

    EXPECT_EQ(0, align.First()->alignedAddress);
    EXPECT_EQ((std::array<int, 16>{ 1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), align.First()->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign4Bytes, Align_single_byte_unaligned_address)
{
    std::array<uint8_t, 1> data = { 1 };
    align.Align(1, data);

    EXPECT_EQ(0, align.First()->alignedAddress);
    EXPECT_EQ((std::array<int, 4>{ 0xff, 1, 0xff, 0xff }), align.First()->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign16Bytes, Align_single_byte_unaligned_address)
{
    std::array<uint8_t, 1> data = { 1 };
    align.Align(1, data);

    EXPECT_EQ(0, align.First()->alignedAddress);
    EXPECT_EQ((std::array<int, 16>{ 0xff, 1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), align.First()->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign4Bytes, Align_two_byte_unaligned_address)
{
    std::array<uint8_t, 2> data = { 1, 2 };
    align.Align(3, data);

    EXPECT_EQ(0, align.First()->alignedAddress);
    EXPECT_EQ((std::array<int, 4>{ 0xff, 0xff, 0xff, 1 }), align.First()->data);

    const services::FlashAlign::Chunk* next = align.Next();
    EXPECT_TRUE(nullptr != next);
    EXPECT_EQ(4, next->alignedAddress);
    EXPECT_EQ((std::array<int, 4>{ 0x2, 0xff, 0xff, 0xff }), next->data);

    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign16Bytes, Align_two_byte_unaligned_address)
{
    std::array<uint8_t, 2> data = { 1, 2 };
    align.Align(15, data);

    EXPECT_EQ(0, align.First()->alignedAddress);
    EXPECT_EQ((std::array<int, 16>{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 }), align.First()->data);

    const services::FlashAlign::Chunk* next = align.Next();
    EXPECT_TRUE(nullptr != next);
    EXPECT_EQ(16, next->alignedAddress);
    EXPECT_EQ((std::array<int, 16>{ 0x2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), next->data);

    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign4Bytes, Align_multiple_bytes_aligned_address)
{
    std::array<uint8_t, 12> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    align.Align(200, data);

    EXPECT_EQ(200, align.First()->alignedAddress);
    EXPECT_EQ(data, align.First()->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign16Bytes, Align_multiple_bytes_aligned_address)
{
    std::array<uint8_t, 48> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8 };
    align.Align(192, data);

    EXPECT_EQ(192, align.First()->alignedAddress);
    EXPECT_EQ(data, align.First()->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign4Bytes, Align_multiple_bytes_unaligned_address)
{
    std::array<uint8_t, 12> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    align.Align(201, data);

    EXPECT_EQ(200, align.First()->alignedAddress);

    EXPECT_EQ((std::array<int, 4>{ 0xff, 1, 2, 3 }), align.First()->data);

    const services::FlashAlign::Chunk* next = align.Next();
    EXPECT_EQ(204, next->alignedAddress);
    EXPECT_EQ((std::array<int, 8>{ 4, 5, 6, 7, 8, 9, 10, 11 }), next->data);

    next = align.Next();
    EXPECT_EQ(212, next->alignedAddress);
    EXPECT_EQ((std::array<int, 4>{ 12, 0xff, 0xff, 0xff }), next->data);
    EXPECT_EQ(nullptr, align.Next());
}

TEST_F(FlashAlign16Bytes, Align_multiple_bytes_unaligned_address)
{
    std::array<uint8_t, 48> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8 };
    align.Align(193, data);

    EXPECT_EQ(192, align.First()->alignedAddress);

    EXPECT_EQ((std::array<int, 16>{ 0xff, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5 }), align.First()->data);

    const services::FlashAlign::Chunk* next = align.Next();
    EXPECT_EQ(208, next->alignedAddress);
    EXPECT_EQ((std::array<int, 32>{ 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7 }), next->data);

    next = align.Next();
    EXPECT_EQ(240, next->alignedAddress);
    EXPECT_EQ((std::array<int, 16>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), next->data);
    EXPECT_EQ(nullptr, align.Next());
}
