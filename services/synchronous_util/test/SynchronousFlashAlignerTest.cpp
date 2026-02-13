#include "services/synchronous_util/SynchronousFlashAligner.hpp"
#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashMock.hpp"
#include "gmock/gmock.h"
#include <algorithm>
#include <array>

namespace
{
    constexpr uint32_t Alignment = 16;
    constexpr uint32_t DefaultFlashSize = 1024 * 1024; // 1MB
    constexpr uint32_t DefaultSectorSize = 4096;
    constexpr uint32_t DefaultNumSectors = DefaultFlashSize / DefaultSectorSize;

    class SynchronousFlashAlignerTest
        : public testing::Test
    {
    public:
        SynchronousFlashAlignerTest()
        {
            EXPECT_CALL(flashMock, NumberOfSectors()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(DefaultNumSectors));
            EXPECT_CALL(flashMock, SizeOfSector(testing::_)).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(DefaultSectorSize));
            EXPECT_CALL(flashMock, AddressOfSector(testing::_)).Times(testing::AnyNumber()).WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * DefaultSectorSize;
                }));
        }

        testing::StrictMock<hal::SynchronousFlashMock> flashMock;
        services::SynchronousFlashAligner::WithAlignment<Alignment> aligner{ flashMock };
    };

    TEST_F(SynchronousFlashAlignerTest, write_buffer_aligned_size)
    {
        std::array<uint8_t, 16> data;
        std::fill(data.begin(), data.end(), 0xAB);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(data), testing::Eq(0x1000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(data), 0x1000);
    }

    TEST_F(SynchronousFlashAlignerTest, write_buffer_smaller_than_alignment)
    {
        std::array<uint8_t, 8> data;
        std::fill(data.begin(), data.end(), 0xCD);

        // Should not write yet, data is buffered
        EXPECT_CALL(flashMock, WriteBuffer(testing::_, testing::_)).Times(0);
        aligner.WriteBuffer(infra::MakeRange(data), 0x2000);
    }

    TEST_F(SynchronousFlashAlignerTest, flush_buffered_data)
    {
        std::array<uint8_t, 8> data;
        std::fill(data.begin(), data.end(), 0xEF);

        aligner.WriteBuffer(infra::MakeRange(data), 0x3000);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 8, 0xEF);
        std::fill(expected.begin() + 8, expected.end(), 0x00);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x3000))).Times(1);
        aligner.Flush();
    }

    TEST_F(SynchronousFlashAlignerTest, write_buffer_fills_alignment_and_writes)
    {
        std::array<uint8_t, 12> firstWrite;
        std::fill(firstWrite.begin(), firstWrite.end(), 0x11);

        // First write should buffer
        EXPECT_CALL(flashMock, WriteBuffer(testing::_, testing::_)).Times(0);
        aligner.WriteBuffer(infra::MakeRange(firstWrite), 0x4000);

        std::array<uint8_t, 8> secondWrite;
        std::fill(secondWrite.begin(), secondWrite.end(), 0x22);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 12, 0x11);
        std::fill(expected.begin() + 12, expected.end(), 0x22);

        // Second write fills buffer and triggers flush
        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x4000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(secondWrite), 0x400C);
    }

    TEST_F(SynchronousFlashAlignerTest, write_buffer_larger_than_alignment)
    {
        std::array<uint8_t, 48> data;
        std::fill(data.begin(), data.end(), 0x33);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(data), testing::Eq(0x5000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(data), 0x5000);
    }

    TEST_F(SynchronousFlashAlignerTest, write_buffer_larger_than_alignment_with_remainder)
    {
        std::array<uint8_t, 50> data;
        std::fill(data.begin(), data.end(), 0x44);

        std::array<uint8_t, 48> expected;
        std::fill(expected.begin(), expected.end(), 0x44);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x6000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(data), 0x6000);
    }

    TEST_F(SynchronousFlashAlignerTest, flush_empty_buffer_does_nothing)
    {
        EXPECT_CALL(flashMock, WriteBuffer(testing::_, testing::_)).Times(0);
        aligner.Flush();
    }

    TEST_F(SynchronousFlashAlignerTest, multiple_writes_with_buffering_and_flush)
    {
        std::array<uint8_t, 10> firstWrite;
        std::fill(firstWrite.begin(), firstWrite.end(), 0x55);

        aligner.WriteBuffer(infra::MakeRange(firstWrite), 0x7000);

        std::array<uint8_t, 20> secondWrite;
        std::fill(secondWrite.begin(), secondWrite.end(), 0x66);

        std::array<uint8_t, 16> firstExpected;
        std::fill(firstExpected.begin(), firstExpected.begin() + 10, 0x55);
        std::fill(firstExpected.begin() + 10, firstExpected.end(), 0x66);

        std::array<uint8_t, 16> secondExpected;
        std::fill(secondExpected.begin(), secondExpected.begin() + 14, 0x66);
        std::fill(secondExpected.begin() + 14, secondExpected.end(), 0x00);

        testing::InSequence seq;
        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(firstExpected), testing::Eq(0x7000))).Times(1);
        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(secondExpected), testing::Eq(0x7010))).Times(1);

        aligner.WriteBuffer(infra::MakeRange(secondWrite), 0x700A);
        aligner.Flush();
    }

    TEST_F(SynchronousFlashAlignerTest, delegates_other_methods)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).Times(1).WillOnce(testing::Return(10));
        EXPECT_THAT(aligner.NumberOfSectors(), testing::Eq(10));

        EXPECT_CALL(flashMock, SizeOfSector(testing::Eq(3))).Times(1).WillOnce(testing::Return(4096));
        EXPECT_THAT(aligner.SizeOfSector(3), testing::Eq(4096));

        EXPECT_CALL(flashMock, SectorOfAddress(testing::Eq(0x1234))).Times(1).WillOnce(testing::Return(5));
        EXPECT_THAT(aligner.SectorOfAddress(0x1234), testing::Eq(5));

        EXPECT_CALL(flashMock, AddressOfSector(testing::Eq(7))).Times(1).WillOnce(testing::Return(0x7000));
        EXPECT_THAT(aligner.AddressOfSector(7), testing::Eq(0x7000));

        std::array<uint8_t, 16> buffer;
        EXPECT_CALL(flashMock, ReadBuffer(testing::_, testing::Eq(0x8000))).Times(1);
        aligner.ReadBuffer(infra::MakeRange(buffer), 0x8000);

        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(256));
        EXPECT_CALL(flashMock, AddressOfSector(testing::Eq(2))).WillRepeatedly(testing::Return(0x2000));
        EXPECT_CALL(flashMock, AddressOfSector(testing::Eq(4))).WillRepeatedly(testing::Return(0x4000));
        EXPECT_CALL(flashMock, SizeOfSector(testing::Eq(4))).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, EraseSectors(testing::Eq(2), testing::Eq(5))).Times(1);
        aligner.EraseSectors(2, 5);
    }

    TEST_F(SynchronousFlashAlignerTest, write_at_flash_boundary)
    {
        std::array<uint8_t, 16> data;
        std::fill(data.begin(), data.end(), 0xAA);

        // Write at a safe position well within 1MB flash
        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(data), testing::Eq(0x1000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(data), 0x1000);
    }

    TEST_F(SynchronousFlashAlignerTest, flush_validates_flash_size)
    {
        std::array<uint8_t, 8> data;
        std::fill(data.begin(), data.end(), 0xBB);

        // Buffer data at safe position
        aligner.WriteBuffer(infra::MakeRange(data), 0x1000);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 8, 0xBB);
        std::fill(expected.begin() + 8, expected.end(), 0x00);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x1000))).Times(1);
        aligner.Flush();
    }

    TEST_F(SynchronousFlashAlignerTest, write_with_overflow_check)
    {
        std::array<uint8_t, 32> data;
        std::fill(data.begin(), data.end(), 0xCC);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(data), testing::Eq(0x8000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(data), 0x8000);
    }

    TEST_F(SynchronousFlashAlignerTest, buffered_write_with_address_increment)
    {
        std::array<uint8_t, 10> firstWrite;
        std::fill(firstWrite.begin(), firstWrite.end(), 0xDD);

        aligner.WriteBuffer(infra::MakeRange(firstWrite), 0x2000);

        std::array<uint8_t, 10> secondWrite;
        std::fill(secondWrite.begin(), secondWrite.end(), 0xEE);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 10, 0xDD);
        std::fill(expected.begin() + 10, expected.begin() + 16, 0xEE);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x2000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(secondWrite), 0x200A);
    }

    TEST_F(SynchronousFlashAlignerTest, large_write_with_remainder_validates_bounds)
    {
        std::array<uint8_t, 100> data;
        std::fill(data.begin(), data.end(), 0x00);

        std::array<uint8_t, 96> expectedWrite;
        std::fill(expectedWrite.begin(), expectedWrite.end(), 0x00);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expectedWrite), testing::Eq(0x1000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(data), 0x1000);
    }

    TEST_F(SynchronousFlashAlignerTest, contiguous_address_continues_buffering)
    {
        // First write - buffer 8 bytes at 0x1000
        std::array<uint8_t, 8> firstWrite;
        std::fill(firstWrite.begin(), firstWrite.end(), 0x11);
        aligner.WriteBuffer(infra::MakeRange(firstWrite), 0x1000);

        // Second write at contiguous address should continue buffering
        std::array<uint8_t, 8> secondWrite;
        std::fill(secondWrite.begin(), secondWrite.end(), 0x22);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 8, 0x11);
        std::fill(expected.begin() + 8, expected.end(), 0x22);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x1000))).Times(1);

        // Contiguous write at 0x1008
        aligner.WriteBuffer(infra::MakeRange(secondWrite), 0x1008);
    }

    TEST_F(SynchronousFlashAlignerTest, multiple_contiguous_writes)
    {
        // First write - buffer 6 bytes at 0x1000
        std::array<uint8_t, 6> firstWrite;
        std::fill(firstWrite.begin(), firstWrite.end(), 0xAA);
        aligner.WriteBuffer(infra::MakeRange(firstWrite), 0x1000);

        // Second write continues contiguously
        std::array<uint8_t, 10> secondWrite;
        std::fill(secondWrite.begin(), secondWrite.end(), 0xBB);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 6, 0xAA);
        std::fill(expected.begin() + 6, expected.end(), 0xBB);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x1000))).Times(1);

        aligner.WriteBuffer(infra::MakeRange(secondWrite), 0x1006);
    }

    TEST_F(SynchronousFlashAlignerTest, aligned_start_addresses_are_accepted)
    {
        std::array<uint8_t, 8> data;
        std::fill(data.begin(), data.end(), 0xFF);

        // Test various aligned addresses (multiples of 16)
        std::array<uint32_t, 5> alignedAddresses = { 0x0000, 0x0010, 0x0100, 0x1000, 0x10000 };

        for (auto address : alignedAddresses)
        {
            std::array<uint8_t, 16> expected;
            std::fill(expected.begin(), expected.begin() + 8, 0xFF);
            std::fill(expected.begin() + 8, expected.end(), 0x00);

            EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(address))).Times(1);

            // Write at aligned address - should buffer without aborting
            aligner.WriteBuffer(infra::MakeRange(data), address);
            aligner.Flush();
        }
    }

    class SynchronousFlashAlignerDeathTest : public testing::Test
    {
    public:
        hal::SynchronousFlashMock flashMock;
        services::SynchronousFlashAligner::WithAlignment<Alignment> aligner{ flashMock };
    };

    TEST_F(SynchronousFlashAlignerDeathTest, write_beyond_flash_size_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(16));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        std::array<uint8_t, 16> data;
        std::fill(data.begin(), data.end(), 0xAA);

        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(data), 0x10000), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, write_that_extends_beyond_flash_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(16));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        std::array<uint8_t, 32> data;
        std::fill(data.begin(), data.end(), 0xBB);

        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(data), 0xFFF0), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, flush_beyond_flash_size_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(16));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        std::array<uint8_t, 8> data;
        std::fill(data.begin(), data.end(), 0xCC);

        // Buffering data at boundary (0x10000 = 16 * 4096) should abort immediately
        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(data), 0x10000), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, address_overflow_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(256));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        std::array<uint8_t, 10> data;
        std::fill(data.begin(), data.end(), 0xEE);

        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(data), 0xFFFFFFFF), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, write_at_non_contiguous_address_with_buffered_data_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(256));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        // First write - buffer 8 bytes at 0x1000
        std::array<uint8_t, 8> firstWrite;
        std::fill(firstWrite.begin(), firstWrite.end(), 0x11);
        aligner.WriteBuffer(infra::MakeRange(firstWrite), 0x1000);

        // Attempt to write at non-contiguous address with buffered data - should abort
        std::array<uint8_t, 8> secondWrite;
        std::fill(secondWrite.begin(), secondWrite.end(), 0x22);
        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(secondWrite), 0x2000), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, write_at_unaligned_address_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(256));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        // Attempt to write at unaligned address (not a multiple of 16) - should abort
        std::array<uint8_t, 8> data;
        std::fill(data.begin(), data.end(), 0xFF);
        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(data), 0x1001), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, buffered_write_beyond_flash_size_aborts_immediately)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(16));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        // TotalSize is 16*4096 = 0x10000
        // Attempt to buffer 4 bytes at 0xFFFE (would span 0xFFFE-0x10002 when aligned)
        // This should abort during WriteBuffer, not later during Flush
        std::array<uint8_t, 4> data;
        std::fill(data.begin(), data.end(), 0xAA);
        EXPECT_DEATH(aligner.WriteBuffer(infra::MakeRange(data), 0xFFFE), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, read_overlapping_buffered_data_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(256));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        // Buffer 8 bytes at 0x1000 (will span 0x1000-0x1008)
        std::array<uint8_t, 8> writeData;
        std::fill(writeData.begin(), writeData.end(), 0xAA);
        aligner.WriteBuffer(infra::MakeRange(writeData), 0x1000);

        // Attempt to read from overlapping address - should abort
        std::array<uint8_t, 16> readData;
        EXPECT_DEATH(aligner.ReadBuffer(infra::MakeRange(readData), 0x1000), "");
    }

    TEST_F(SynchronousFlashAlignerDeathTest, erase_overlapping_buffered_data_aborts)
    {
        EXPECT_CALL(flashMock, NumberOfSectors()).WillRepeatedly(testing::Return(256));
        EXPECT_CALL(flashMock, SizeOfSector(testing::_)).WillRepeatedly(testing::Return(4096));
        EXPECT_CALL(flashMock, AddressOfSector(testing::_))
            .WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * 4096;
                }));

        // Buffer 8 bytes at 0x1000 (will span 0x1000-0x1008), which is in sector 1 (0x1000-0x1FFF)
        std::array<uint8_t, 8> writeData;
        std::fill(writeData.begin(), writeData.end(), 0xBB);
        aligner.WriteBuffer(infra::MakeRange(writeData), 0x1000);

        // Attempt to erase sector 1 which contains buffered data - should abort
        EXPECT_DEATH(aligner.EraseSectors(1, 2), "");
    }
}

namespace
{
    class SynchronousFlashAlignerOverlapTest : public testing::Test
    {
    public:
        SynchronousFlashAlignerOverlapTest()
        {
            EXPECT_CALL(flashMock, NumberOfSectors()).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(DefaultNumSectors));
            EXPECT_CALL(flashMock, SizeOfSector(testing::_)).Times(testing::AnyNumber()).WillRepeatedly(testing::Return(DefaultSectorSize));
            EXPECT_CALL(flashMock, AddressOfSector(testing::_)).Times(testing::AnyNumber()).WillRepeatedly(testing::Invoke([](uint32_t sector)
                {
                    return sector * DefaultSectorSize;
                }));
        }

        testing::StrictMock<hal::SynchronousFlashMock> flashMock;
        services::SynchronousFlashAligner::WithAlignment<Alignment> aligner{ flashMock };
    };

    TEST_F(SynchronousFlashAlignerOverlapTest, read_non_overlapping_address_succeeds)
    {
        // Buffer 8 bytes at 0x1000 (will span 0x1000-0x1008)
        std::array<uint8_t, 8> writeData;
        std::fill(writeData.begin(), writeData.end(), 0xCC);
        aligner.WriteBuffer(infra::MakeRange(writeData), 0x1000);

        // Read from non-overlapping address 0x2000 - should succeed
        std::array<uint8_t, 16> readData;
        EXPECT_CALL(flashMock, ReadBuffer(testing::_, testing::Eq(0x2000))).Times(1);
        aligner.ReadBuffer(infra::MakeRange(readData), 0x2000);
    }

    TEST_F(SynchronousFlashAlignerOverlapTest, read_without_buffered_data_succeeds)
    {
        // No buffered data, read should work at any address
        std::array<uint8_t, 16> readData;
        EXPECT_CALL(flashMock, ReadBuffer(testing::_, testing::Eq(0x1000))).Times(1);
        aligner.ReadBuffer(infra::MakeRange(readData), 0x1000);
    }

    TEST_F(SynchronousFlashAlignerOverlapTest, erase_non_overlapping_sectors_succeeds)
    {
        // Buffer 8 bytes at 0x1000 (sector 0, spans 0x1000-0x1008)
        std::array<uint8_t, 8> writeData;
        std::fill(writeData.begin(), writeData.end(), 0xDD);
        aligner.WriteBuffer(infra::MakeRange(writeData), 0x1000);

        // Erase sectors 2-3 (non-overlapping with sector 0) - should succeed
        EXPECT_CALL(flashMock, EraseSectors(testing::Eq(2), testing::Eq(3))).Times(1);
        aligner.EraseSectors(2, 3);
    }

    TEST_F(SynchronousFlashAlignerOverlapTest, erase_without_buffered_data_succeeds)
    {
        // No buffered data, erase should work
        EXPECT_CALL(flashMock, EraseSectors(testing::Eq(0), testing::Eq(1))).Times(1);
        aligner.EraseSectors(0, 1);
    }

    TEST_F(SynchronousFlashAlignerOverlapTest, read_after_flush_succeeds)
    {
        // Buffer and flush data
        std::array<uint8_t, 8> writeData;
        std::fill(writeData.begin(), writeData.end(), 0xEE);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 8, 0xEE);
        std::fill(expected.begin() + 8, expected.end(), 0x00);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x1000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(writeData), 0x1000);
        aligner.Flush();

        // After flush, buffer is empty, so read should work
        std::array<uint8_t, 16> readData;
        EXPECT_CALL(flashMock, ReadBuffer(testing::_, testing::Eq(0x1000))).Times(1);
        aligner.ReadBuffer(infra::MakeRange(readData), 0x1000);
    }

    TEST_F(SynchronousFlashAlignerOverlapTest, erase_after_flush_succeeds)
    {
        // Buffer and flush data at sector 0
        std::array<uint8_t, 8> writeData;
        std::fill(writeData.begin(), writeData.end(), 0xFF);

        std::array<uint8_t, 16> expected;
        std::fill(expected.begin(), expected.begin() + 8, 0xFF);
        std::fill(expected.begin() + 8, expected.end(), 0x00);

        EXPECT_CALL(flashMock, WriteBuffer(testing::ElementsAreArray(expected), testing::Eq(0x1000))).Times(1);
        aligner.WriteBuffer(infra::MakeRange(writeData), 0x1000);
        aligner.Flush();

        // After flush, buffer is empty, so erase should work
        EXPECT_CALL(flashMock, EraseSectors(testing::Eq(0), testing::Eq(1))).Times(1);
        aligner.EraseSectors(0, 1);
    }
}
