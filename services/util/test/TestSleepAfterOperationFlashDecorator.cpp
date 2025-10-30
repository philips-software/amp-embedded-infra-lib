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

    TEST_F(SleepAfterOperationFlashDecoratorTest, size_of_sector_forwards_args_and_result)
    {
        uint32_t size = 4096;
        EXPECT_CALL(flash, SizeOfSector(5)).WillOnce(::testing::Return(size));
        EXPECT_EQ(decorator.SizeOfSector(5), size);
    }

    TEST_F(SleepAfterOperationFlashDecoratorTest, sector_of_address_forwards_args_and_result)
    {
        uint32_t sector = 7;
        EXPECT_CALL(flash, SectorOfAddress(0x1000)).WillOnce(::testing::Return(sector));
        EXPECT_EQ(decorator.SectorOfAddress(0x1000), sector);
    }

    TEST_F(SleepAfterOperationFlashDecoratorTest, address_of_sector_forwards_args_and_result)
    {
        uint32_t address = 0x2000;
        EXPECT_CALL(flash, AddressOfSector(3)).WillOnce(::testing::Return(address));
        EXPECT_EQ(decorator.AddressOfSector(3), address);
    }
}
