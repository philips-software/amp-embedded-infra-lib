#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/util/FlashRegion.hpp"

class FlashRegionTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    FlashRegionTest()
        : flashStub(4, 4)
        , regionImpl(flashStub, 2, 2)
        , region(&regionImpl)
    {
        flashStub.sectors = std::vector<std::vector<uint8_t>>{ { 0xff }, { 0xff, 0xff }, { 0xff, 0xff, 0xff }, { 0xff, 0xff, 0xff, 0xff } };
    }

    hal::FlashStub flashStub;
    services::FlashRegion regionImpl;
    hal::Flash* region;
};

TEST_F(FlashRegionTest, Construct)
{
    EXPECT_EQ(2, region->NumberOfSectors());
    EXPECT_EQ(3, region->SizeOfSector(0));
    EXPECT_EQ(4, region->SizeOfSector(1));
}

TEST_F(FlashRegionTest, WriteBuffer)
{
    region->WriteBuffer(std::vector<uint8_t>{ 0x01 }, region->AddressOfSector(0), infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 0x01, 0xff, 0xff }), flashStub.sectors[2]);
}

TEST_F(FlashRegionTest, WriteBufferAtEnd)
{
    region->WriteBuffer(std::vector<uint8_t>{ 0x01 }, region->AddressOfSector(1) + 3, infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0x01 }), flashStub.sectors[3]);
}

TEST_F(FlashRegionTest, ReadBuffer)
{
    flashStub.sectors[2] = std::vector<uint8_t>{ 0x01, 0xff, 0xff };
    uint8_t buffer{ 0xff };
    region->ReadBuffer(infra::MakeByteRange(buffer), region->AddressOfSector(0), infra::emptyFunction);
    EXPECT_EQ(0x01, buffer);
}

TEST_F(FlashRegionTest, ReadBufferAtEnd)
{
    flashStub.sectors[3] = std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0x01 };
    uint8_t buffer{ 0xff };
    region->ReadBuffer(infra::MakeByteRange(buffer), region->AddressOfSector(1) + 3, infra::emptyFunction);
    EXPECT_EQ(0x01, buffer);
}

TEST_F(FlashRegionTest, EraseSector)
{
    flashStub.sectors[3] = std::vector<uint8_t>{ 0x01, 0x02, 0x03, 0x04 };
    region->EraseSector(1, infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff }), flashStub.sectors[3]);
}

TEST_F(FlashRegionTest, EraseSectors)
{
    flashStub.sectors[2] = std::vector<uint8_t>{ 0x01, 0x02, 0x03 };
    flashStub.sectors[3] = std::vector<uint8_t>{ 0x01, 0x02, 0x03, 0x04 };
    region->EraseSectors(0, 2, infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff }), flashStub.sectors[2]);
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff }), flashStub.sectors[3]);
}
