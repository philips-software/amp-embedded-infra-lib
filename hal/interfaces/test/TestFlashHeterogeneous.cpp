#include "hal/interfaces/FlashHeterogeneous.hpp"
#include "gtest/gtest.h"
#include <cstdint>

class FlashHeterogeneousImpl
    : public hal::FlashHeterogeneous
{
    using hal::FlashHeterogeneous::FlashHeterogeneous;

    void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override
    {}

    void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override
    {}

    void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override
    {}
};

class FlashHeterogeneousTest
    : public testing::Test
{
public:
    std::array<uint32_t, 3> sectorSizes{ 3, 4, 8 };
    FlashHeterogeneousImpl flash{ infra::MakeRange(sectorSizes) };
};

TEST_F(FlashHeterogeneousTest, number_of_sectors)
{
    ASSERT_EQ(3, flash.NumberOfSectors());
}

TEST_F(FlashHeterogeneousTest, size_of_sector)
{
    ASSERT_EQ(3, flash.SizeOfSector(0));
    ASSERT_EQ(4, flash.SizeOfSector(1));
    ASSERT_EQ(8, flash.SizeOfSector(2));
}

TEST_F(FlashHeterogeneousTest, sector_of_address)
{
    ASSERT_EQ(0, flash.SectorOfAddress(0));
    ASSERT_EQ(0, flash.SectorOfAddress(1));

    ASSERT_EQ(1, flash.SectorOfAddress(3));
    ASSERT_EQ(1, flash.SectorOfAddress(6));

    ASSERT_EQ(2, flash.SectorOfAddress(7));
    ASSERT_EQ(3, flash.SectorOfAddress(15));
}

TEST_F(FlashHeterogeneousTest, address_of_sector)
{
    ASSERT_EQ(0, flash.AddressOfSector(0));
    ASSERT_EQ(3, flash.AddressOfSector(1));
    ASSERT_EQ(7, flash.AddressOfSector(2));
}
