#include "hal/generic/SynchronousFlashInMemory.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SynchronousFlashInMemoryTest
    : public testing::Test
{
public:
    hal::SynchronousFlashInMemory flash{ 3, 4 };
};

TEST_F(SynchronousFlashInMemoryTest, construction_fills_sectors_with_erased_value)
{
    EXPECT_EQ((std::vector<std::vector<uint8_t>>(3, std::vector<uint8_t>(4, 0xff))), flash.Sectors());
}

TEST_F(SynchronousFlashInMemoryTest, construction_with_custom_erased_value)
{
    hal::SynchronousFlashInMemory zeroErasedFlash{ 2, 2, 0x00 };

    EXPECT_EQ((std::vector<uint8_t>{ 0, 0, 0, 0 }), zeroErasedFlash.Contents());
}

TEST_F(SynchronousFlashInMemoryTest, write_spanning_sectors)
{
    flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2, 3, 4, 5, 6 }), 3);

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{ { 0xff, 0xff, 0xff, 1 }, { 2, 3, 4, 5 }, { 6, 0xff, 0xff, 0xff } }), flash.Sectors());
}

TEST_F(SynchronousFlashInMemoryTest, write_only_clears_bits)
{
    flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 0xf0 }), 0);
    flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 0x3c }), 0);

    EXPECT_EQ(0x30, flash.Sectors()[0][0]);
}

TEST_F(SynchronousFlashInMemoryTest, read_spanning_sectors)
{
    flash.Sectors() = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 } };

    std::vector<uint8_t> data(6);
    flash.ReadBuffer(infra::MakeRange(data), 3);

    EXPECT_EQ((std::vector<uint8_t>{ 4, 5, 6, 7, 8, 9 }), data);
}

TEST_F(SynchronousFlashInMemoryTest, read_and_write_with_heterogeneous_sectors)
{
    flash.Sectors() = { { 0xff }, { 0xff, 0xff, 0xff }, { 0xff, 0xff } };

    flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2, 3, 4, 5, 6 }), 0);

    std::vector<uint8_t> data(4);
    flash.ReadBuffer(infra::MakeRange(data), 1);

    EXPECT_EQ((std::vector<uint8_t>{ 2, 3, 4, 5 }), data);
    EXPECT_EQ((std::vector<std::vector<uint8_t>>{ { 1 }, { 2, 3, 4 }, { 5, 6 } }), flash.Sectors());
}

TEST_F(SynchronousFlashInMemoryTest, erase_sectors_restores_erased_value)
{
    hal::SynchronousFlashInMemory zeroErasedFlash{ 3, 2, 0x00 };
    zeroErasedFlash.Sectors() = { { 1, 2 }, { 3, 4 }, { 5, 6 } };

    zeroErasedFlash.EraseSectors(1, 3);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 0, 0, 0, 0 }), zeroErasedFlash.Contents());
}

TEST_F(SynchronousFlashInMemoryTest, contents_concatenates_sectors)
{
    flash.Sectors() = { { 1, 2 }, { 3 }, { 4, 5, 6 } };

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4, 5, 6 }), flash.Contents());
}

TEST_F(SynchronousFlashInMemoryTest, memory_mapped_range_covers_whole_flash)
{
    hal::MemoryMappedSynchronousFlash& memoryMappedFlash = flash;

    EXPECT_EQ(memoryMappedFlash.TotalSize(), memoryMappedFlash.MemoryMappedRange().size());
}

TEST_F(SynchronousFlashInMemoryTest, memory_mapped_range_keeps_its_address)
{
    hal::MemoryMappedSynchronousFlash& memoryMappedFlash = flash;

    auto first = memoryMappedFlash.MemoryMappedRange();
    memoryMappedFlash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1 }), 0);

    EXPECT_EQ(first.begin(), memoryMappedFlash.MemoryMappedRange().begin());
}

TEST_F(SynchronousFlashInMemoryTest, memory_mapped_range_reflects_writes_and_erases)
{
    hal::MemoryMappedSynchronousFlash& memoryMappedFlash = flash;

    memoryMappedFlash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2, 3, 4, 5, 6 }), 3);
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 1, 2, 3, 4, 5, 6, 0xff, 0xff, 0xff }), infra::MakeVector(memoryMappedFlash.MemoryMappedRange()));

    memoryMappedFlash.EraseSectors(0, 1);
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff, 2, 3, 4, 5, 6, 0xff, 0xff, 0xff }), infra::MakeVector(memoryMappedFlash.MemoryMappedRange()));
}

TEST_F(SynchronousFlashInMemoryTest, retained_memory_mapped_range_reflects_writes)
{
    hal::MemoryMappedSynchronousFlash& memoryMappedFlash = flash;
    auto range = memoryMappedFlash.MemoryMappedRange();

    memoryMappedFlash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2, 3, 4, 5, 6 }), 3);

    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 1, 2, 3, 4, 5, 6, 0xff, 0xff, 0xff }), infra::MakeVector(range));
}

TEST_F(SynchronousFlashInMemoryTest, retained_memory_mapped_range_reflects_erases)
{
    hal::MemoryMappedSynchronousFlash& memoryMappedFlash = flash;
    memoryMappedFlash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2, 3, 4, 5, 6 }), 3);
    auto range = memoryMappedFlash.MemoryMappedRange();

    memoryMappedFlash.EraseSectors(0, 1);

    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff, 2, 3, 4, 5, 6, 0xff, 0xff, 0xff }), infra::MakeVector(range));
}

TEST_F(SynchronousFlashInMemoryTest, access_up_to_the_end_is_allowed)
{
    std::vector<uint8_t> data(2);

    flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2 }), 10);
    flash.ReadBuffer(infra::MakeRange(data), 10);
    flash.ReadBuffer(infra::ByteRange(), 12);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2 }), data);
}

#ifndef EMIL_MUTATION_TESTING
TEST_F(SynchronousFlashInMemoryTest, write_beyond_end_aborts)
{
    EXPECT_DEATH(flash.WriteBuffer(infra::MakeRange(std::vector<uint8_t>{ 1, 2 }), 11), "");
}

TEST_F(SynchronousFlashInMemoryTest, read_beyond_end_aborts)
{
    std::vector<uint8_t> data(1);

    EXPECT_DEATH(flash.ReadBuffer(infra::MakeRange(data), 12), "");
}

TEST_F(SynchronousFlashInMemoryTest, erase_beyond_end_aborts)
{
    EXPECT_DEATH(flash.EraseSectors(2, 4), "");
}

TEST_F(SynchronousFlashInMemoryTest, erase_with_reversed_range_aborts)
{
    EXPECT_DEATH(flash.EraseSectors(2, 1), "");
}
#endif
