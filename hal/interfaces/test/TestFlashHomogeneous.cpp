#include "hal/interfaces/FlashHomogeneous.hpp"
#include "gtest/gtest.h"
#include <cstdint>

class FlashHomogeneousImpl
    : public hal::FlashHomogeneous
{
    using hal::FlashHomogeneous::FlashHomogeneous;

    void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override
    {}

    void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override
    {}

    void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override
    {}
};

class FlashHomogeneousTest
    : public testing::Test
{
public:
    FlashHomogeneousImpl flash{ 3, 8 };
};

TEST_F(FlashHomogeneousTest, number_of_sectors)
{
    ASSERT_EQ(flash.NumberOfSectors(), 3);
}

TEST_F(FlashHomogeneousTest, size_of_sector)
{
    ASSERT_EQ(flash.SizeOfSector(0), 8);
    ASSERT_EQ(flash.SizeOfSector(1), 8);
    ASSERT_EQ(flash.SizeOfSector(2), 8);
}

TEST_F(FlashHomogeneousTest, sector_of_address)
{
    ASSERT_EQ(flash.SectorOfAddress(0), 0);
    ASSERT_EQ(flash.SectorOfAddress(1), 0);

    ASSERT_EQ(flash.SectorOfAddress(8), 1);
    ASSERT_EQ(flash.SectorOfAddress(9), 1);

    ASSERT_EQ(flash.SectorOfAddress(16), 2);
}

TEST_F(FlashHomogeneousTest, address_of_sector)
{
    ASSERT_EQ(flash.AddressOfSector(0), 0);
    ASSERT_EQ(flash.AddressOfSector(1), 8);
    ASSERT_EQ(flash.AddressOfSector(2), 16);
}
