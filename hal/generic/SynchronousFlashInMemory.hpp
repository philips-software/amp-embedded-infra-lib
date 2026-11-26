#ifndef HAL_GENERIC_SYNCHRONOUS_FLASH_IN_MEMORY_HPP
#define HAL_GENERIC_SYNCHRONOUS_FLASH_IN_MEMORY_HPP

#include "hal/synchronous_interfaces/MemoryMappedSynchronousFlash.hpp"
#include <cstdint>
#include <vector>

namespace hal
{
    class SynchronousFlashInMemory
        : public hal::MemoryMappedSynchronousFlash
    {
    public:
        SynchronousFlashInMemory(uint32_t numberOfSectors, uint32_t sizeOfEachSector, uint8_t erasedValue = 0xff);

        uint32_t NumberOfSectors() const override;
        uint32_t SizeOfSector(uint32_t sectorIndex) const override;

        uint32_t SectorOfAddress(uint32_t address) const override;
        uint32_t AddressOfSector(uint32_t sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex) override;

        // The sectors are not stored contiguously, so every call refreshes a copy; its address stays the same while the flash size does not change
        infra::ConstByteRange MemoryMappedRange() const override;

        std::vector<std::vector<uint8_t>>& Sectors();
        const std::vector<std::vector<uint8_t>>& Sectors() const;
        std::vector<uint8_t> Contents() const;

    private:
        bool IsInRange(uint32_t address, std::size_t size) const;
        infra::ByteRange SectorRangeAt(uint32_t address, std::size_t maxSize);

        std::vector<std::vector<uint8_t>> sectors;
        uint8_t erasedValue;
        mutable std::vector<uint8_t> memoryMappedSnapshot;
    };
}

#endif
