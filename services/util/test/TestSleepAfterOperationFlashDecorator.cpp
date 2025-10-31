#include "hal/interfaces/test_doubles/FlashMock.hpp"
#include "hal/interfaces/test_doubles/SleepableMock.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/SleepAfterOperationFlashDecorator.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace services
{
    template<typename T>
    class SleepAfterOperationFlashDecoratorTest
        : public testing::Test
    {
    public:
        SleepAfterOperationFlashDecoratorTest()
        {
            ON_CALL(flash, WriteBuffer(testing::_, testing::_, testing::_)).WillByDefault(testing::InvokeArgument<2>());
            ON_CALL(flash, ReadBuffer(testing::_, testing::_, testing::_)).WillByDefault(testing::InvokeArgument<2>());
            ON_CALL(flash, EraseSectors(testing::_, testing::_, testing::_)).WillByDefault(testing::InvokeArgument<2>());

            ON_CALL(sleepable, Sleep(testing::_)).WillByDefault(testing::InvokeArgument<0>());
            ON_CALL(sleepable, Wake(testing::_)).WillByDefault(testing::InvokeArgument<0>());
        }

        testing::StrictMock<hal::CleanFlashMockBase<T>> flash;
        testing::StrictMock<hal::SleepableMock> sleepable;
        SleepAfterOperationFlashDecoratorBase<T> decorator{ flash, sleepable };

        std::array<uint8_t, 32> dummyArray;
    };

    typedef testing::Types<uint32_t, uint64_t> FlashTypes;
    TYPED_TEST_SUITE(SleepAfterOperationFlashDecoratorTest, FlashTypes);

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, number_of_sectors_forwards_result)
    {
        uint32_t nr = 42;
        EXPECT_CALL(this->flash, NumberOfSectors()).WillOnce(testing::Return(nr));
        EXPECT_EQ(this->decorator.NumberOfSectors(), nr);
    }

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, size_of_sector_forwards_args_and_result)
    {
        uint32_t size = 4096;
        EXPECT_CALL(this->flash, SizeOfSector(5)).WillOnce(testing::Return(size));
        EXPECT_EQ(this->decorator.SizeOfSector(5), size);
    }

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, sector_of_address_forwards_args_and_result)
    {
        uint32_t sector = 7;
        EXPECT_CALL(this->flash, SectorOfAddress(0x1000)).WillOnce(testing::Return(sector));
        EXPECT_EQ(this->decorator.SectorOfAddress(0x1000), sector);
    }

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, address_of_sector_forwards_args_and_result)
    {
        uint32_t address = 0x2000;
        EXPECT_CALL(this->flash, AddressOfSector(3)).WillOnce(testing::Return(address));
        EXPECT_EQ(this->decorator.AddressOfSector(3), address);
    }

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, write_buffer_does_wake_write_sleep)
    {
        uint32_t address = 42;
        auto buffer = infra::MakeConstByteRange(this->dummyArray);

        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, WriteBuffer(buffer, address, testing::_));
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        infra::VerifyingFunction<void()> onDone;
        this->decorator.WriteBuffer(buffer, address, onDone);
    }

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, read_buffer_does_wake_read_sleep)
    {
        uint32_t address = 42;
        auto buffer = infra::MakeByteRange(this->dummyArray);

        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, ReadBuffer(buffer, address, testing::_));
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        infra::VerifyingFunction<void()> onDone;
        this->decorator.ReadBuffer(buffer, address, onDone);
    }

    TYPED_TEST(SleepAfterOperationFlashDecoratorTest, erase_sectors_does_wake_erase_sleep)
    {
        uint32_t beginIndex = 2;
        uint32_t endIndex = 5;

        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, EraseSectors(beginIndex, endIndex, testing::_));
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        infra::VerifyingFunction<void()> onDone;
        this->decorator.EraseSectors(beginIndex, endIndex, onDone);
    }
}
