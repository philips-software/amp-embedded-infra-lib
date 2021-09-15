#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_HPP

#include "infra/util/ByteRange.hpp"
#include <cstdint>
#include <utility>

namespace hal
{
    class SynchronousFlash
    {
    public:
        using Range = std::pair<uint32_t, uint32_t>;

        SynchronousFlash() = default;
        SynchronousFlash(const SynchronousFlash& other) = delete;
        SynchronousFlash& operator=(const SynchronousFlash& other) = delete;

    protected:
        ~SynchronousFlash() = default;

    public:
        virtual uint32_t NumberOfSectors() const = 0;
        virtual uint32_t SizeOfSector(uint32_t sectorIndex) const = 0;
        uint32_t TotalSize() const;

        virtual uint32_t SectorOfAddress(uint32_t address) const = 0;
        virtual uint32_t AddressOfSector(uint32_t sectorIndex) const = 0;
        uint32_t StartOfSector(uint32_t address) const;
        uint32_t AddressOffsetInSector(uint32_t address) const;
        uint32_t StartOfNextSector(uint32_t address) const;
        uint32_t StartOfPreviousSector(uint32_t address) const;
        uint32_t StartOfNextSectorCyclical(uint32_t address) const;
        uint32_t StartOfPreviousSectorCyclical(uint32_t address) const;
        bool AtStartOfSector(uint32_t address) const;

        virtual void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) = 0;
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address) = 0;

        void EraseSector(uint32_t sectorIndex);
        virtual void EraseSectors(uint32_t beginIndex, uint32_t endIndex) = 0; // Erases sectors in de range [beginIndex, endIndex)
        void EraseAll();
    };
}

#endif
