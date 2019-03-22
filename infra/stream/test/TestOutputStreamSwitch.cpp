#include "gtest/gtest.h"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include <cstdint>

TEST(OutputStreamSwitchTest, StreamToTextStream)
{
    infra::ByteOutputStream::WithStorage<2> stream;

    stream << infra::text << uint8_t(12);
    EXPECT_EQ((std::array<uint8_t, 2>{{ '1', '2' }}), stream.Writer().Processed());
}

TEST(OutputStreamSwitchTest, TextStreamToStream)
{
    infra::StringOutputStream::WithStorage<2> stream;

    stream << infra::data << uint8_t(012);
    EXPECT_EQ("\012", stream.Storage());
}

TEST(OutputStreamSwitchTest, StreamToTextStreamToStream)
{
    infra::ByteOutputStream::WithStorage<3> stream;

    stream << infra::text << uint8_t(12) << infra::data << uint8_t(12);
    EXPECT_EQ((std::array<uint8_t, 3>{{ '1', '2', 12 }}), stream.Writer().Processed());
}

TEST(OutputStreamSwitchTest, StreamFromBoundedString)
{
    infra::BoundedString::WithStorage<10> s = "abcd";
    infra::StringOutputStream stream(s);

    stream << uint8_t(12) << infra::data << uint8_t('a');

    EXPECT_EQ("abcd12a", s);
}
