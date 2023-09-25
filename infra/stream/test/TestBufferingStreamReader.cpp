#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "gmock/gmock.h"

TEST(BufferingStreamReaderTest, Extract)
{
    infra::BoundedDeque<uint8_t>::WithMaxSize<4> buffer{ std::initializer_list<uint8_t>{ 1, 2 } };
    testing::StrictMock<infra::StreamReaderWithRewindingMock> input;
    infra::BufferingStreamReader reader{ buffer, input };

    EXPECT_CALL(input, Empty()).WillOnce(testing::Return(true));
}
