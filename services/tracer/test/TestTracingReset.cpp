#include "hal/interfaces/test_doubles/ResetMock.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/TracingReset.hpp"
#include "gmock/gmock.h"

TEST(TracingResetTest, reset_traces_reason)
{
    hal::ResetMock reset;
    infra::StringOutputStream::WithStorage<64> stream;
    services::Tracer tracer(stream);
    services::TracingReset tracingReset(reset, tracer);

    EXPECT_CALL(reset, ResetModule(testing::_));

    tracingReset.ResetModule("Upgrade");
    EXPECT_EQ("\r\nReset module: Upgrade\r\n...\r\n...", stream.Storage());
}
