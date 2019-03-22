#include "hal/synchronous_interfaces/SynchronousFlash.hpp"

namespace hal
{
    uint32_t SynchronousFlash::TotalSize() const
    {
        return AddressOfSector(NumberOfSectors() - 1) + SizeOfSector(NumberOfSectors() - 1);
    }

    uint32_t SynchronousFlash::StartOfSector(uint32_t address) const
    {
        return AddressOfSector(SectorOfAddress(address));
    }

    uint32_t SynchronousFlash::AddressOffsetInSector(uint32_t address) const
    {
        return address - StartOfSector(address);
    }

    uint32_t SynchronousFlash::StartOfNextSector(uint32_t address) const
    {
        return AddressOfSector(SectorOfAddress(address) + 1);
    }

    uint32_t SynchronousFlash::StartOfPreviousSector(uint32_t address) const
    {
        return AddressOfSector(SectorOfAddress(address) - 1);
    }

    uint32_t SynchronousFlash::StartOfNextSectorCyclical(uint32_t address) const
    {
        uint32_t sectorIndex = SectorOfAddress(address) + 1;
        if (sectorIndex == NumberOfSectors())
            sectorIndex = 0;
        return AddressOfSector(sectorIndex);
    }

    uint32_t SynchronousFlash::StartOfPreviousSectorCyclical(uint32_t address) const
    {
        uint32_t sectorIndex = SectorOfAddress(address);
        if (sectorIndex == 0)
            sectorIndex = NumberOfSectors();
        return AddressOfSector(sectorIndex - 1);
    }

    bool SynchronousFlash::AtStartOfSector(uint32_t address) const
    {
        return StartOfSector(address) == address;
    }

    void SynchronousFlash::EraseSector(uint32_t sectorIndex)
    {
        EraseSectors(sectorIndex, sectorIndex + 1);
    }

    void SynchronousFlash::EraseAll()
    {
        EraseSectors(0, NumberOfSectors());
    }
}
