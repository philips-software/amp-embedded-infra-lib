#include "hal/interfaces/Flash.hpp"

namespace hal
{
    uint32_t Flash::TotalSize() const
    {
        return AddressOfSector(NumberOfSectors() - 1) + SizeOfSector(NumberOfSectors() - 1);
    }

    uint32_t Flash::StartOfSector(uint32_t address) const
    {
        return AddressOfSector(SectorOfAddress(address));
    }

    uint32_t Flash::AddressOffsetInSector(uint32_t address) const
    {
        return address - StartOfSector(address);
    }

    uint32_t Flash::StartOfNextSector(uint32_t address) const
    {
        return AddressOfSector(SectorOfAddress(address) + 1);
    }

    uint32_t Flash::StartOfPreviousSector(uint32_t address) const
    {
        return AddressOfSector(SectorOfAddress(address) - 1);
    }

    uint32_t Flash::StartOfNextSectorCyclical(uint32_t address) const
    {
        uint32_t sectorIndex = SectorOfAddress(address) + 1;
        if (sectorIndex == NumberOfSectors())
            sectorIndex = 0;
        return AddressOfSector(sectorIndex);
    }

    uint32_t Flash::StartOfPreviousSectorCyclical(uint32_t address) const
    {
        uint32_t sectorIndex = SectorOfAddress(address);
        if (sectorIndex == 0)
            sectorIndex = NumberOfSectors();
        return AddressOfSector(sectorIndex - 1);
    }

    bool Flash::AtStartOfSector(uint32_t address) const
    {
        return StartOfSector(address) == address;
    }

    void Flash::EraseSector(uint32_t sectorIndex, infra::Function<void()> onDone)
    {
        EraseSectors(sectorIndex, sectorIndex + 1, onDone);
    }

    void Flash::EraseAll(infra::Function<void()> onDone)
    {
        EraseSectors(0, NumberOfSectors(), onDone);
    }
}
