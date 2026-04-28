#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/Tracer.hpp"
#include "gmock/gmock.h"

class TracerDisabledTestImpl
    : public services::TracerToStream
{
public:
    using services::TracerToStream::TracerToStream;
};

TEST(TracerDisabledTest, trace_does_not_insert_header)
{
    infra::StringOutputStream::WithStorage<32> stream;
    TracerDisabledTestImpl tracer(stream);

    tracer.Trace();

    EXPECT_EQ("", stream.Storage());
}

TEST(TracerDisabledTest, trace_does_not_stream_text)
{
    infra::StringOutputStream::WithStorage<32> stream;
    TracerDisabledTestImpl tracer(stream);

    tracer.Trace() << "Text";

    EXPECT_EQ("", stream.Storage());
}

TEST(TracerDisabledTest, continue_does_not_stream_text)
{
    infra::StringOutputStream::WithStorage<32> stream;
    TracerDisabledTestImpl tracer(stream);

    tracer.Continue() << "Text";

    EXPECT_EQ("", stream.Storage());
}

TEST(TracerDisabledTest, writer_from_continue_does_not_write)
{
    infra::StringOutputStream::WithStorage<32> stream;
    TracerDisabledTestImpl tracer(stream);

    infra::StreamWriter& writer = tracer.Continue().Writer();
    infra::ConstByteRange data = infra::MakeStringByteRange("Text");
    infra::StreamErrorPolicy errorPolicy;
    writer.Insert(data, errorPolicy);

    EXPECT_EQ("", stream.Storage());
}

TEST(TracerDisabledTest, stream_from_continue_does_not_write)
{
    infra::StringOutputStream::WithStorage<32> stream;
    TracerDisabledTestImpl tracer(stream);

    infra::TextOutputStream returnedStream = tracer.Continue();
    returnedStream << "Text";

    EXPECT_EQ("", stream.Storage());
}
