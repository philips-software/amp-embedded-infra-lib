#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "infra/syntax/Asn1Formatter.hpp"
#include "infra/stream/ByteOutputStream.hpp"

namespace
{
    static std::array<uint8_t, 2> shortData = { 0xAB, 0xBA };
}

TEST(Asn1ObjectFormatter, construction_results_in_empty_object)
{
    infra::ByteOutputStream::WithStorage<4> stream;
    auto marker = stream.SaveMarker();

    infra::Asn1Formatter formatter(stream);

    EXPECT_EQ(false, formatter.Failed());
    EXPECT_EQ(0, stream.ProcessedBytesSince(marker));
}

TEST(Asn1ObjectFormatter, add_uint8)
{
    infra::ByteOutputStream::WithStorage<3> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.Add(uint8_t(0xAB));

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x01, 0xAB));
}

TEST(Asn1ObjectFormatter, add_uint32)
{
    infra::ByteOutputStream::WithStorage<6> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.Add(uint32_t(0x01ABCDEF));

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x04, 0x01, 0xAB, 0xCD, 0xEF));
}

TEST(Asn1ObjectFormatter, add_int32)
{
    infra::ByteOutputStream::WithStorage<6> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.Add(int32_t(-1));

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF));
}

TEST(Asn1ObjectFormatter, add_serial)
{
    infra::ByteOutputStream::WithStorage<6> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 4> serial = { 1, 2, 3, 4 };

    formatter.AddSerial(serial);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x04, 0x01, 0x02, 0x03, 0x04));
}

TEST(Asn1ObjectFormatter, add_bignum)
{
    infra::ByteOutputStream::WithStorage<8> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 6> data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };

    formatter.AddBigNumber(data);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x06, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01));
}

TEST(Asn1ObjectFormatter, add_bignum_zero)
{
    infra::ByteOutputStream::WithStorage<2> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 1> data = { 0x00 };

    formatter.AddBigNumber(data);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x00));
}

TEST(Asn1ObjectFormatter, add_bignum_using_minumum_amount_of_bytes)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 6> data = { 0x00, 0x01, 0x01, 0x00, 0x00, 0x00 };

    formatter.AddBigNumber(data);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x03, 0x01, 0x01, 0x00));
}

TEST(Asn1ObjectFormatter, add_bignum_zero_pad_when_high_bit_is_set)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 2> data = { 0xFF, 0x80 };

    formatter.AddBigNumber(data);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x03, 0x00, 0x80, 0xFF));
}

TEST(Asn1ObjectFormatter, add_context_specific)
{
    const uint8_t context = 1;

    infra::ByteOutputStream::WithStorage<4> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddContextSpecific(context, shortData);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x20 | 0x80 | context, 0x02, 0xAB, 0xBA));
}

TEST(Asn1ObjectFormatter, add_object_id)
{
    infra::ByteOutputStream::WithStorage<4> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddObjectId(shortData);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x06, 0x02, 0xAB, 0xBA));
}

TEST(Asn1ObjectFormatter, add_bit_string)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddBitString(shortData);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x03, 0x03, 0x00, 0xAB, 0xBA));
}

TEST(Asn1ObjectFormatter, add_printable_string)
{
    infra::ByteOutputStream::WithStorage<4> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddPrintableString(shortData);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x13, 0x02, 0xAB, 0xBA));
}

TEST(Asn1ObjectFormatter, add_utc_time)
{
    infra::ByteOutputStream::WithStorage<15> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddUtcTime(2017, 1, 1, 12, 15, 00);

    std::array<uint8_t, 15> expected = { 0x17, 0x0D, '1', '7', '0', '1', '0', '1', '1', '2', '1', '5', '0', '0', 'Z' };

    EXPECT_EQ(expected, stream.Storage());
}

TEST(Asn1ObjectFormatter, add_generalized_time)
{
    infra::ByteOutputStream::WithStorage<17> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddGeneralizedTime(2050, 1, 1, 12, 15, 00);

    std::array<uint8_t, 17> expected = { 0x18, 0x0F, '2', '0', '5', '0', '0', '1', '0', '1', '1', '2', '1', '5', '0', '0', 'Z' };

    EXPECT_EQ(expected, stream.Storage());
}

TEST(Asn1ObjectFormatter, add_time)
{    
    {
        infra::ByteOutputStream::WithStorage<32> stream;
        infra::Asn1Formatter formatter(stream);
        formatter.AddTime(1950, 1, 1, 1, 1, 1);
        EXPECT_EQ(0x17, stream.Storage()[0]);
    }

    {
        infra::ByteOutputStream::WithStorage<32> stream;
        infra::Asn1Formatter formatter(stream);
        formatter.AddTime(2050, 1, 1, 1, 1, 1);
        EXPECT_EQ(0x18, stream.Storage()[0]);
    }
}

TEST(Asn1ObjectFormatter, add_empty_optional)
{
    infra::ByteOutputStream::WithStorage<2> stream;
    infra::Asn1Formatter formatter(stream);

    formatter.AddOptional<uint32_t>(infra::none);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x05, 0x00));
}

TEST(Asn1ObjectFormatter, add_non_empty_optional)
{
    infra::ByteOutputStream::WithStorage<3> stream;
    infra::Asn1Formatter formatter(stream);

    infra::Optional<uint8_t> value(infra::inPlace, 0xAB);
    formatter.AddOptional<uint8_t>(value);

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x02, 0x01, 0xAB));
}

TEST(Asn1ObjectFormatter, start_sequence)
{
    infra::ByteOutputStream::WithStorage<8> stream;
    infra::Asn1Formatter formatter(stream);
    {
        auto sequence = formatter.StartSequence();
        sequence.Add(0xFFFFFFFF);
    }

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x20 | 0x10, 0x06, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF));
}

TEST(Asn1ObjectFormatter, start_set)
{
    infra::ByteOutputStream::WithStorage<8> stream;
    infra::Asn1Formatter formatter(stream);
    {
        auto sequence = formatter.StartSet();
        sequence.Add(0xFFFFFFFF);
    }

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x20 | 0x11, 0x06, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF));
}

TEST(Asn1ObjectFormatter, start_context_specific)
{
    const uint8_t context = 3;

    infra::ByteOutputStream::WithStorage<8> stream;
    infra::Asn1Formatter formatter(stream);
    {
        auto sequence = formatter.StartContextSpecific(context);
        sequence.Add(0xFFFFFFFF);
    }

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x20 | 0x80 | context, 0x06, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF));
}

TEST(Asn1ObjectFormatter, start_bit_string)
{
    infra::ByteOutputStream::WithStorage<9> stream;
    infra::Asn1Formatter formatter(stream);
    {
        auto sequence = formatter.StartBitString();
        sequence.Add(0xFFFFFFFF);
    }

    ASSERT_THAT(stream.Storage(), testing::ElementsAre(0x03, 0x07, 0x00, 0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF));
}

TEST(Asn1ObjectFormatter, add_medium_sized_object)
{
    infra::ByteOutputStream::WithStorage<131> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 128> mediumSizedObject;
    formatter.AddPrintableString(mediumSizedObject);

    EXPECT_EQ(0x13, stream.Storage()[0]);
    EXPECT_EQ(0x81, stream.Storage()[1]);
    EXPECT_EQ(0x80, stream.Storage()[2]);
}

TEST(Asn1ObjectFormatter, add_large_object)
{
    infra::ByteOutputStream::WithStorage<260> stream;
    infra::Asn1Formatter formatter(stream);

    std::array<uint8_t, 256> largeObject;
    formatter.AddPrintableString(largeObject);

    EXPECT_EQ(0x13, stream.Storage()[0]);
    EXPECT_EQ(0x82, stream.Storage()[1]);
    EXPECT_EQ(0x01, stream.Storage()[2]);
    EXPECT_EQ(0x00, stream.Storage()[3]);
}
