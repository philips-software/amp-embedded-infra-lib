#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashMock.hpp"
#include "services/synchronous_util/SynchronousFlashDelegate.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <array>

namespace
{
    class SynchronousFlashDelegateTest
        : public ::testing::Test
    {
    public:
        testing::StrictMock<hal::SynchronousFlashMock> flashMock;
        services::SynchronousFlashDelegateBase delegate{ flashMock };
    };

    TEST_F(SynchronousFlashDelegateTest, number_of_sectors)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillOnce(testing::Return(42));

        EXPECT_THAT(delegate.NumberOfSectors(), testing::Eq(42));
    }

    TEST_F(SynchronousFlashDelegateTest, size_of_sector)
    {
        EXPECT_CALL(flashMock, SizeOfSector(5)).WillOnce(testing::Return(8192));

        EXPECT_THAT(delegate.SizeOfSector(5), testing::Eq(8192));
    }

    TEST_F(SynchronousFlashDelegateTest, sector_of_address)
    {
        EXPECT_CALL(flashMock, SectorOfAddress(0x1000)).WillOnce(testing::Return(2));

        EXPECT_THAT(delegate.SectorOfAddress(0x1000), testing::Eq(2));
    }

    TEST_F(SynchronousFlashDelegateTest, address_of_sector)
    {
        EXPECT_CALL(flashMock, AddressOfSector(3)).WillOnce(testing::Return(0x3000));

        EXPECT_THAT(delegate.AddressOfSector(3), testing::Eq(0x3000));
    }

    TEST_F(SynchronousFlashDelegateTest, write_buffer)
    {
        std::array<uint8_t, 16> data{};
        std::fill(data.begin(), data.end(), uint8_t{ 0xAB });

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(data), 0x100)).Times(1);

        delegate.WriteBuffer(infra::MakeConstByteRange(data), 0x100);
    }

    TEST_F(SynchronousFlashDelegateTest, read_buffer)
    {
        std::array<uint8_t, 16> expectedData{};
        std::fill(expectedData.begin(), expectedData.end(), uint8_t{ 0xCD });

        EXPECT_CALL(flashMock, ReadBuffer(testing::SizeIs(16), 0x200))
            .WillOnce(testing::Invoke([expectedData](infra::ByteRange buffer, uint32_t)
                {
                    std::copy(expectedData.begin(), expectedData.end(), buffer.begin());
                }));

        std::array<uint8_t, 16> readData{};
        delegate.ReadBuffer(infra::MakeByteRange(readData), 0x200);

        EXPECT_THAT(readData, testing::ElementsAreArray(expectedData));
    }

    TEST_F(SynchronousFlashDelegateTest, erase_sectors)
    {
        EXPECT_CALL(flashMock, EraseSectors(2, 5)).Times(1);

        delegate.EraseSectors(2, 5);
    }
}
