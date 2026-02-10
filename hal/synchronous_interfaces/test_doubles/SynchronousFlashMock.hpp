#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_MOCK_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_MOCK_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include <gmock/gmock.h>

namespace hal
{
    class SynchronousFlashMock : public SynchronousFlash
    {
    public:
        MOCK_METHOD(uint32_t, NumberOfSectors, (), (const, override));
        MOCK_METHOD(uint32_t, SizeOfSector, (uint32_t sectorIndex), (const, override));
        MOCK_METHOD(uint32_t, SectorOfAddress, (uint32_t address), (const, override));
        MOCK_METHOD(uint32_t, AddressOfSector, (uint32_t sectorIndex), (const, override));
        MOCK_METHOD(void, WriteBuffer, (infra::ConstByteRange buffer, uint32_t address), (override));
        MOCK_METHOD(void, ReadBuffer, (infra::ByteRange buffer, uint32_t address), (override));
        MOCK_METHOD(void, EraseSectors, (uint32_t beginIndex, uint32_t endIndex), (override));
    };
}

#endif
