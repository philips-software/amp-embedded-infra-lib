#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/tracer/Tracer.hpp"
#include "gmock/gmock.h"

class TracerTestImpl
    : public services::TracerToStream
{
public:
    using services::TracerToStream::TracerToStream;

    MOCK_METHOD0(InsertHeader, void());
};

TEST(TracerTest, trace_inserts_header)
{
    infra::StringOutputStream::WithStorage<32> stream;
    testing::StrictMock<TracerTestImpl> tracer(stream);

    EXPECT_CALL(tracer, InsertHeader());
    tracer.Trace();
}

TEST(TracerTest, trace_streams_text)
{
    infra::StringOutputStream::WithStorage<32> stream;
    testing::StrictMock<TracerTestImpl> tracer(stream);
    EXPECT_CALL(tracer, InsertHeader());

    tracer.Trace() << "Text";

    EXPECT_EQ("\r\nText", stream.Storage());
}

TEST(GlobalTracerTest, global_tracer_streams_text)
{
    infra::StringOutputStream::WithStorage<32> stream;
    testing::StrictMock<TracerTestImpl> tracer(stream);
    services::SetGlobalTracerInstance(tracer);
    EXPECT_CALL(tracer, InsertHeader());

    services::GlobalTracer().Trace() << "Text";

    EXPECT_EQ("\r\nText", stream.Storage());
}

TEST(TracerPrefixedTest, prefix_is_inserted_before_message)
{
    infra::StringOutputStream::WithStorage<64> stream;
    services::TracerToStream baseTracer(stream);
    services::TracerPrefixed tracer("prefix: ", baseTracer);

    tracer.Trace() << "message";

    EXPECT_EQ("\r\nprefix: message", stream.Storage());
}

TEST(TracerPrefixedTest, empty_prefix_leaves_output_unchanged)
{
    infra::StringOutputStream::WithStorage<64> stream;
    services::TracerToStream baseTracer(stream);
    services::TracerPrefixed tracer("", baseTracer);

    tracer.Trace() << "message";

    EXPECT_EQ("\r\nmessage", stream.Storage());
}

TEST(TracerPrefixedTest, prefix_is_repeated_on_each_trace)
{
    infra::StringOutputStream::WithStorage<64> stream;
    services::TracerToStream baseTracer(stream);
    services::TracerPrefixed tracer("[mod] ", baseTracer);

    tracer.Trace() << "first";
    tracer.Trace() << "second";

    EXPECT_EQ("\r\n[mod] first\r\n[mod] second", stream.Storage());
}
