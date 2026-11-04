#include "infra/syntax/Asn1.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <vector>

TEST(Asn1Test, get_integer)
{
    std::array<uint8_t, 3> data = { { 0x02, 0x01, 0xab } };
    infra::Asn1Sequence sequence(data);

    EXPECT_EQ(infra::ConstByteRange(data.data() + 2, data.data() + 3), sequence.front().Integer());
}

TEST(Asn1Test, get_integer_1_byte_length)
{
    std::array<uint8_t, 4> data = { { 0x02, 0x81, 0x01, 0xab } };
    infra::Asn1Sequence sequence(data);

    EXPECT_EQ(infra::ConstByteRange(data.data() + 3, data.data() + 4), sequence.front().Integer());
}

TEST(Asn1Test, get_integer_2_byte_length)
{
    std::array<uint8_t, 5> data = { { 0x02, 0x82, 0x00, 0x01, 0xab } };
    infra::Asn1Sequence sequence(data);

    EXPECT_EQ(infra::ConstByteRange(data.data() + 4, data.data() + 5), sequence.front().Integer());
}

TEST(Asn1Test, get_sequence)
{
    std::array<uint8_t, 8> data = { { 0x30, 0x06, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF } };
    infra::Asn1Sequence sequence(data);

    EXPECT_EQ(infra::Asn1Sequence(infra::ConstByteRange(data.data() + 2, data.data() + 8)), sequence.begin()->Sequence());
    EXPECT_FALSE(infra::Asn1Sequence(infra::ConstByteRange(data.data() + 2, data.data() + 8)) != sequence.begin()->Sequence());
}

TEST(Asn1Test, get_second_integer)
{
    std::array<uint8_t, 6> data = { { 0x02, 0x01, 0xab, 0x02, 0x01, 0xcd } };
    infra::Asn1Sequence sequence(data);

    EXPECT_EQ(infra::ConstByteRange(data.data() + 5, data.data() + 6), std::next(sequence.begin())->Integer());
    EXPECT_EQ(infra::ConstByteRange(data.data() + 5, data.data() + 6), sequence[1].Integer());
}

TEST(Asn1Test, iterator_end)
{
    std::array<uint8_t, 3> data = { { 0x02, 0x01, 0xab } };
    infra::Asn1Sequence sequence(data);

    EXPECT_TRUE(sequence.end() == infra::Asn1SequenceIterator());
    EXPECT_FALSE(sequence.end() != infra::Asn1SequenceIterator());
}

TEST(Asn1Test, dereference_iterator)
{
    std::array<uint8_t, 3> data = { { 0x02, 0x01, 0xab } };
    infra::Asn1Sequence sequence(data);

    EXPECT_EQ(infra::ConstByteRange(data.data() + 2, data.data() + 3), (*sequence.begin()).Integer());
    EXPECT_EQ(infra::ConstByteRange(data.data() + 2, data.data() + 3), (*static_cast<const infra::Asn1SequenceIterator&>(sequence.begin())).Integer());
    EXPECT_EQ(infra::ConstByteRange(data.data() + 2, data.data() + 3), sequence.begin().operator->()->Integer());
    EXPECT_EQ(infra::ConstByteRange(data.data() + 2, data.data() + 3), static_cast<const infra::Asn1SequenceIterator&>(sequence.begin()).operator->()->Integer());
}

TEST(Asn1Test, increment_iterator)
{
    std::array<uint8_t, 6> data = { { 0x02, 0x01, 0xab, 0x02, 0x01, 0xcd } };
    infra::Asn1Sequence sequence(data);

    infra::Asn1SequenceIterator iterator = sequence.begin();
    EXPECT_EQ(sequence[1].Integer(), (++iterator)->Integer());

    infra::Asn1SequenceIterator iterator2 = sequence.begin();
    EXPECT_EQ(sequence[0].Integer(), (iterator2++)->Integer());
    EXPECT_EQ(sequence[1].Integer(), (iterator2++)->Integer());
}

TEST(Asn1Test, increment_iterator_past_empty_value)
{
    std::array<uint8_t, 5> data = { { 0x05, 0x00, 0x02, 0x01, 0xab } };
    infra::Asn1Sequence sequence(data);

    EXPECT_THAT(sequence[1].Integer(), testing::Eq(infra::ConstByteRange(data.data() + 4, data.data() + 5)));
    EXPECT_THAT(std::next(sequence.begin(), 2), testing::Eq(sequence.end()));
}

TEST(Asn1Test, iteration_ends_at_malformed_value)
{
    std::array<uint8_t, 4> data = { { 0x02, 0x01, 0xab, 0x02 } };
    infra::Asn1Sequence sequence(data);

    EXPECT_THAT(sequence.front().Integer(), testing::Eq(infra::ConstByteRange(data.data() + 2, data.data() + 3)));
    EXPECT_THAT(std::next(sequence.begin()), testing::Eq(sequence.end()));
}

class Asn1MalformedTest
    : public testing::TestWithParam<std::vector<uint8_t>>
{};

TEST_P(Asn1MalformedTest, malformed_value_results_in_empty_sequence)
{
    infra::Asn1Sequence sequence(infra::MakeRange(GetParam()));

    EXPECT_THAT(sequence.begin(), testing::Eq(sequence.end()));
}

INSTANTIATE_TEST_SUITE_P(Asn1MalformedTest, Asn1MalformedTest,
    testing::Values(
        std::vector<uint8_t>{},
        std::vector<uint8_t>{ 0x02 },
        std::vector<uint8_t>{ 0x02, 0x81 },
        std::vector<uint8_t>{ 0x02, 0x82, 0x00 },
        std::vector<uint8_t>{ 0x02, 0x05, 0x00 },
        std::vector<uint8_t>{ 0x02, 0x81, 0x02, 0x00 },
        std::vector<uint8_t>{ 0x02, 0x80, 0x00 },
        std::vector<uint8_t>{ 0x02, 0x83, 0x00, 0x00, 0x01, 0xab }));
