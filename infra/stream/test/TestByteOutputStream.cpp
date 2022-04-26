#include "gtest/gtest.h"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/OverwriteStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"

TEST(ByteOutputStreamTest, StreamToRange)
{
    struct From
    {
        uint8_t a;
        uint8_t b;
    } from = { 0, 1 };

    std::array<uint8_t, 4> to = { 2, 3, 4, 5 };

    infra::ByteOutputStream stream(to);
    stream << from;

    EXPECT_EQ((std::array<uint8_t, 4>{ { 0, 1, 4, 5 } }), to);
    EXPECT_EQ((std::vector<uint8_t>{ 4, 5 }), stream.Writer().Remaining());
    EXPECT_EQ((std::vector<uint8_t>{ 0, 1 }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, StreamFromMemoryRange)
{
    std::array<uint8_t, 2> from = { 0, 1 };
    std::array<uint8_t, 4> buffer = { 2, 3, 4, 5 };

    infra::ByteOutputStream stream(buffer);
    stream << infra::ByteRange(from);

    EXPECT_EQ((std::array<uint8_t, 4>{ { 0, 1, 4, 5 } }), buffer);
}

TEST(ByteOutputStreamTest, WithStorage)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    stream << uint8_t(1) << uint8_t(2) << uint8_t(3);

    EXPECT_EQ((std::array<uint8_t, 3>{ { 1, 2, 3 } }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, Reset)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    stream << uint8_t(1) << uint8_t(2);
    stream.Writer().Reset();
    stream << uint8_t(3);

    EXPECT_EQ((std::array<uint8_t, 1>{ { 3 } }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, Reset_with_new_range)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    stream << uint8_t(1) << uint8_t(2);
    std::array<uint8_t, 4> newRange{};
    stream.Writer().Reset(newRange);
    stream << uint8_t(3);

    EXPECT_EQ(3, newRange[0]);
}

TEST(ByteOutputStreamTest, reserve_type)
{
    infra::ByteOutputStream::WithStorage<5> stream;
    stream << uint8_t(1);
    auto reservedSpace = stream.Writer().Reserve<uint8_t>(stream.ErrorPolicy());
    stream << uint8_t(3);
    reservedSpace = uint8_t(2);

    EXPECT_EQ((std::array<uint8_t, 3>{ { 1, 2, 3 } }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, reserve_type_without_space)
{
    infra::ByteOutputStream::WithStorage<2> stream(infra::softFail);
    stream << uint8_t(1);
    auto reservedSpace = stream.Writer().Reserve<uint32_t>(stream.ErrorPolicy());
    reservedSpace = uint32_t(32);

    EXPECT_TRUE(stream.Failed());
}

TEST(ByteOutputStreamTest, stream_to_saved_point)
{
    infra::ByteOutputStream::WithStorage<64> stream;
    stream << uint8_t(1);
    auto marker = stream.SaveMarker();
    stream << uint8_t(3);

    {
        EXPECT_EQ(1, stream.ProcessedBytesSince(marker));
        infra::SavedMarkerDataStream savedStream(stream, marker);
        savedStream << uint8_t(2);
    }

    EXPECT_EQ((std::array<uint8_t, 3>{ { 1, 2, 3 } }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, overwrite_on_saved_point)
{
    infra::ByteOutputStream::WithStorage<64> stream;
    stream << uint8_t(1);
    auto marker = stream.SaveMarker();
    stream << uint8_t(2);
    stream << uint8_t(3);

    {
        EXPECT_EQ(2, stream.ProcessedBytesSince(marker));
        infra::OverwriteDataStream overwriteStream(stream, marker);
        overwriteStream << uint8_t(4);
    }

    EXPECT_EQ((std::array<uint8_t, 3>{ { 1, 4, 3 } }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, stream_to_nested_saved_point)
{
    infra::ByteOutputStream::WithStorage<64> stream;
    stream << uint8_t(1);
    auto marker = stream.SaveMarker();
    stream << uint8_t(5);

    {
        infra::SavedMarkerDataStream savedStream(stream, marker);
        savedStream << uint8_t(2);
        auto nestedMarker = savedStream.SaveMarker();
        savedStream << uint8_t(4);

        {
            EXPECT_EQ(1, savedStream.ProcessedBytesSince(nestedMarker));
            infra::SavedMarkerDataStream nestedSavedStream(savedStream, nestedMarker);
            nestedSavedStream << uint8_t(3);
        }
    }

    EXPECT_EQ((std::array<uint8_t, 5>{ { 1, 2, 3, 4, 5 } }), stream.Writer().Processed());
}

TEST(ByteOutputStreamTest, ByteOutputStream_available_returns_streamWriter_available)
{
    infra::ByteOutputStream::WithStorage<64> stream;
    EXPECT_EQ(64, stream.Available());

    uint32_t integer = 33;
    stream << integer;

    EXPECT_EQ(60, stream.Available());
}

TEST(ByteOutputStreamTest, overflow_does_not_overwrite_buffer)
{
    std::array<uint8_t, 4> from = { 0, 1, 2, 3 };
    std::array<uint8_t, 4> buffer = { 4, 5, 6, 7 };

    infra::ByteOutputStream stream(infra::Head(infra::MakeRange(buffer), 2), infra::noFail);
    stream << infra::ByteRange(from);

    EXPECT_EQ((std::array<uint8_t, 4>{ { 0, 1, 6, 7 } }), buffer);
}
