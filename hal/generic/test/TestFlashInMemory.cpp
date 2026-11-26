#include "hal/generic/FlashInMemory.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class FlashInMemoryTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    hal::FlashInMemory flash{ 3, 4 };
};

TEST_F(FlashInMemoryTest, construction_fills_sectors_with_erased_value)
{
    EXPECT_EQ((std::vector<std::vector<uint8_t>>(3, std::vector<uint8_t>(4, 0xff))), flash.Sectors());
}

TEST_F(FlashInMemoryTest, construction_with_custom_erased_value)
{
    hal::FlashInMemory zeroErasedFlash{ 2, 2, 0x00 };

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{ { 0, 0 }, { 0, 0 } }), zeroErasedFlash.Sectors());
}

TEST_F(FlashInMemoryTest, write_spanning_sectors_only_clears_bits)
{
    infra::MockCallback<void()> done;
    EXPECT_CALL(done, callback());

    flash.Sectors()[1] = { 0x0f, 0xff, 0xff, 0xff };
    flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 0xf2, 3 }), 3, [&done]
        {
            done.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{ { 0xff, 0xff, 0xff, 1 }, { 0x02, 3, 0xff, 0xff }, { 0xff, 0xff, 0xff, 0xff } }), flash.Sectors());
}

TEST_F(FlashInMemoryTest, read_spanning_sectors)
{
    infra::MockCallback<void()> done;
    EXPECT_CALL(done, callback());

    flash.Sectors() = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 } };

    std::vector<uint8_t> data(6);
    flash.ReadBuffer(infra::MakeRange(data), 3, [&done]
        {
            done.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 4, 5, 6, 7, 8, 9 }), data);
}

TEST_F(FlashInMemoryTest, erase_sectors_restores_erased_value)
{
    infra::MockCallback<void()> done;
    EXPECT_CALL(done, callback());

    hal::FlashInMemory zeroErasedFlash{ 3, 2, 0x00 };
    zeroErasedFlash.Sectors() = { { 1, 2 }, { 3, 4 }, { 5, 6 } };

    zeroErasedFlash.EraseSectors(1, 3, [&done]
        {
            done.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{ { 1, 2 }, { 0, 0 }, { 0, 0 } }), zeroErasedFlash.Sectors());
}

TEST_F(FlashInMemoryTest, supports_64_bit_addresses)
{
    hal::FlashInMemory64 flash64{ 2, 4 };

    EXPECT_EQ(2u, flash64.NumberOfSectors());
    EXPECT_EQ(4u, flash64.AddressOfSector(1));
    EXPECT_EQ(1u, flash64.SectorOfAddress(5));
}
