#ifndef HAL_INTERFACE_FLASH_HPP
#define HAL_INTERFACE_FLASH_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include <cstdint>

namespace hal
{
    class Flash
    {
    protected:
        Flash() = default;
        Flash(const Flash& other) = delete;
        Flash& operator=(const Flash& other) = delete;
        ~Flash() = default;

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

        virtual void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) = 0;
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) = 0;

        void EraseSector(uint32_t sectorIndex, infra::Function<void()> onDone);
        virtual void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) = 0;  // Erases sectors in de range [beginIndex, endIndex)
        void EraseAll(infra::Function<void()> onDone);
    };
}

#endif
