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
    ASSERT_EQ(flash.NumberOfSectors(), 3);
}

TEST_F(FlashHeterogeneousTest, size_of_sector)
{
    ASSERT_EQ(flash.SizeOfSector(0), 3);
    ASSERT_EQ(flash.SizeOfSector(1), 4);
    ASSERT_EQ(flash.SizeOfSector(2), 8);
}

TEST_F(FlashHeterogeneousTest, sector_of_address)
{
    ASSERT_EQ(flash.SectorOfAddress(0), 0);
    ASSERT_EQ(flash.SectorOfAddress(1), 0);

    ASSERT_EQ(flash.SectorOfAddress(3), 1);
    ASSERT_EQ(flash.SectorOfAddress(6), 1);

    ASSERT_EQ(flash.SectorOfAddress(7), 2);
    ASSERT_EQ(flash.SectorOfAddress(15), 2);
}

TEST_F(FlashHeterogeneousTest, address_of_sector)
{
    ASSERT_EQ(flash.AddressOfSector(0), 0);
    ASSERT_EQ(flash.AddressOfSector(1), 3);
    ASSERT_EQ(flash.AddressOfSector(2), 7);
}
