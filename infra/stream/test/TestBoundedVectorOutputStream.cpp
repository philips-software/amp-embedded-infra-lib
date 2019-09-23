#include "gtest/gtest.h"
#include "infra/stream/OverwriteStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include <limits>

TEST(BoundedVectorOutputStreamTest, stream_byte)
{
    infra::BoundedVectorOutputStream::WithStorage<2> stream;

    stream << uint8_t(12);
    EXPECT_EQ(12, stream.Storage().front());
    EXPECT_EQ(1, stream.Storage().size());
}

TEST(BoundedVectorOutputStreamTest, reserve_type)
{
    infra::BoundedVectorOutputStream::WithStorage<64> stream;
    stream << 'a';
    auto reservedSpace = stream.Writer().Reserve<uint8_t>(stream.ErrorPolicy());
    stream << 'c';
    reservedSpace = 'b';

    EXPECT_EQ('b', stream.Storage()[1]);
}

TEST(BoundedVectorOutputStreamTest, stream_to_saved_point)
{
    infra::BoundedVectorOutputStream::WithStorage<64> stream;
    stream << infra::text << "a";
    auto marker = stream.SaveMarker();
    stream << infra::text << "c";

    {
        EXPECT_EQ(1, stream.ProcessedBytesSince(marker));
        infra::SavedMarkerTextStream savedStream(stream, marker);
        savedStream << 12345;
    }

    EXPECT_EQ("a12345c", infra::ByteRangeAsString(infra::MakeRange(stream.Storage())));
}

TEST(BoundedVectorOutputStreamTest, available_retuns_remaining_space)
{
    infra::BoundedVectorOutputStream::WithStorage<4> sos;
    EXPECT_EQ(4, sos.Available());

    sos << static_cast<uint32_t>(1);
    EXPECT_EQ(0, sos.Available());
}

TEST(BoundedVectorOutputStreamTest, Reset)
{
    infra::BoundedVectorOutputStream::WithStorage<2> stream;

    stream << uint8_t(12);
    stream.Writer().Reset();
    stream << uint8_t(14);

    EXPECT_EQ(14, stream.Storage().front());
}

TEST(BoundedVectorOutputStreamTest, Reset_with_new_storage)
{
    infra::BoundedVectorOutputStream::WithStorage<2> stream;

    stream << uint8_t(12);
    infra::BoundedVector<uint8_t>::WithMaxSize<4> newVector;
    stream.Writer().Reset(newVector);
    stream << uint8_t(14);

    EXPECT_EQ(14, newVector.front());
}
