#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/TracerWithPrefix.hpp"
#include "gmock/gmock.h"

TEST(TracerWithPrefixTest, prefix_is_inserted_before_message)
{
    infra::StringOutputStream::WithStorage<64> stream;
    services::TracerToStream baseTracer(stream);
    services::TracerWithPrefix tracer("prefix: ", baseTracer);

    tracer.Trace() << "message";

    EXPECT_EQ("\r\nprefix: message", stream.Storage());
}

TEST(TracerWithPrefixTest, empty_prefix_leaves_output_unchanged)
{
    infra::StringOutputStream::WithStorage<64> stream;
    services::TracerToStream baseTracer(stream);
    services::TracerWithPrefix tracer("", baseTracer);

    tracer.Trace() << "message";

    EXPECT_EQ("\r\nmessage", stream.Storage());
}

TEST(TracerWithPrefixTest, prefix_is_repeated_on_each_trace)
{
    infra::StringOutputStream::WithStorage<64> stream;
    services::TracerToStream baseTracer(stream);
    services::TracerWithPrefix tracer("[mod] ", baseTracer);

    tracer.Trace() << "first";
    tracer.Trace() << "second";

    EXPECT_EQ("\r\n[mod] first\r\n[mod] second", stream.Storage());
}
