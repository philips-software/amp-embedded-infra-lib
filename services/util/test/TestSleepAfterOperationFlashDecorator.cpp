#include "hal/interfaces/test_doubles/FlashMock.hpp"
#include "services/util/SleepAfterOperationFlashDecorator.hpp"
#include "services/util/test_doubles/SleepableMock.hpp"
#include "gtest/gtest.h"

namespace services
{
    class SleepAfterOperationFlashDecoratorTest : public testing::Test
    {
    public:
        ::testing::StrictMock<hal::CleanFlashMock> flash;
        ::testing::StrictMock<SleepableMock> sleepable;
        SleepAfterOperationFlashDecorator decorator{ flash, sleepable };
    };

    TEST_F(SleepAfterOperationFlashDecoratorTest, number_of_sectors_forwards_result)
    {
        uint32_t nr = 42;
        EXPECT_CALL(flash, NumberOfSectors()).WillOnce(::testing::Return(nr));
        EXPECT_EQ(decorator.NumberOfSectors(), nr);
    }
}
