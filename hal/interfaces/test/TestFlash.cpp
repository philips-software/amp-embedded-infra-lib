#include "hal/interfaces/Flash.hpp"
#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "gtest/gtest.h"

class FlashAddressTest
    : public testing::Test
{
public:
    FlashAddressTest()
        : flash(3, 7)
    {}

    hal::FlashStub flash;
};

TEST_F(FlashAddressTest, StartAddress)
{
    EXPECT_EQ(0, flash.SectorOfAddress(0));
    EXPECT_EQ(0, flash.AddressOffsetInSector(0));
    EXPECT_TRUE(flash.AtStartOfSector(0));
}

TEST_F(FlashAddressTest, AddressOfSector)
{
    EXPECT_EQ(7, flash.AddressOfSector(1));
}

TEST_F(FlashAddressTest, StartOfSector)
{
    EXPECT_EQ(0, flash.StartOfSector(1));
}

TEST_F(FlashAddressTest, StartOfNextSector)
{
    EXPECT_EQ(7, flash.StartOfNextSector(0));
    EXPECT_TRUE(flash.AtStartOfSector(7));
}

TEST_F(FlashAddressTest, StartOfNextSectorMidwayInASector)
{
    EXPECT_EQ(7, flash.StartOfNextSector(1));
    EXPECT_TRUE(flash.AtStartOfSector(7));
}

TEST_F(FlashAddressTest, StartOfPreviousSector)
{
    EXPECT_EQ(0, flash.StartOfPreviousSector(7));
    EXPECT_TRUE(flash.AtStartOfSector(flash.StartOfPreviousSector(7)));
}

TEST_F(FlashAddressTest, StartOfPreviousSectorMidwayInASector)
{
    EXPECT_EQ(0, flash.StartOfPreviousSector(8));
    EXPECT_TRUE(flash.AtStartOfSector(flash.StartOfPreviousSector(8)));
}
