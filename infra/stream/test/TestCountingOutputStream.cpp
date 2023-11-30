#include "infra/stream/CountingOutputStream.hpp"
#include "gmock/gmock.h"

TEST(CountingOutputStreamTest, Insert)
{
    infra::CountingStreamWriter writer;
    infra::StreamErrorPolicy errorPolicy;

    std::array<uint8_t, 4> data;
    writer.Insert(data, errorPolicy);
    EXPECT_EQ(4, writer.Processed());
    EXPECT_EQ(std::numeric_limits<std::size_t>::max(), writer.Available());
}

TEST(CountingOutputStreamTest, save_and_restore)
{
    std::array<uint8_t, 6> saveStateStorage;
    infra::CountingStreamWriter writer(saveStateStorage);
    infra::StreamErrorPolicy errorPolicy;

    std::array<uint8_t, 4> data;
    writer.Insert(data, errorPolicy);

    EXPECT_EQ(4, writer.ConstructSaveMarker());
    EXPECT_EQ(infra::MakeRange(saveStateStorage), writer.SaveState(4));
    writer.Insert(infra::Head(infra::MakeRange(data), 3), errorPolicy);
    EXPECT_EQ(3, writer.GetProcessedBytesSince(4));
    writer.RestoreState(infra::Tail(infra::MakeRange(saveStateStorage), 1)); // Restore 1 out of the 6 bytes
    EXPECT_EQ(4 + 3 + 5, writer.Processed());
}
