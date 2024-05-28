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
    EXPECT_EQ(3, flash.NumberOfSectors());
}

TEST_F(FlashHomogeneousTest, size_of_sector)
{
    EXPECT_EQ(8, flash.SizeOfSector(0));
    EXPECT_EQ(8, flash.SizeOfSector(1));
    EXPECT_EQ(8, flash.SizeOfSector(2));
}

TEST_F(FlashHomogeneousTest, sector_of_address)
{
    EXPECT_EQ(0, flash.SectorOfAddress(0));
    EXPECT_EQ(0, flash.SectorOfAddress(1));

    EXPECT_EQ(1, flash.SectorOfAddress(8));
    EXPECT_EQ(1, flash.SectorOfAddress(15));

    EXPECT_EQ(2, flash.SectorOfAddress(16));
    EXPECT_EQ(3, flash.SectorOfAddress(24));
}

TEST_F(FlashHomogeneousTest, address_of_sector)
{
    EXPECT_EQ(0, flash.AddressOfSector(0));
    EXPECT_EQ(8, flash.AddressOfSector(1));
    EXPECT_EQ(16, flash.AddressOfSector(2));
}
