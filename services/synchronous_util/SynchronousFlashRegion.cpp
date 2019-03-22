#include "services/synchronous_util/SynchronousFlashRegion.hpp"

namespace services
{
    SynchronousFlashRegion::SynchronousFlashRegion(hal::SynchronousFlash& master, uint32_t startSector, uint32_t numberOfSectors)
        : master(master)
        , startSector(startSector)
        , numberOfSectors(numberOfSectors)
        , masterAddressOfStartSector(master.AddressOfSector(startSector))
    {
        assert(startSector + numberOfSectors <= master.NumberOfSectors());
    }

    uint32_t SynchronousFlashRegion::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    uint32_t SynchronousFlashRegion::SizeOfSector(uint32_t sectorIndex) const
    {
        return master.SizeOfSector(startSector + sectorIndex);
    }

    uint32_t SynchronousFlashRegion::SectorOfAddress(uint32_t address) const
    {
        return master.SectorOfAddress(masterAddressOfStartSector + address) - startSector;
    }

    uint32_t SynchronousFlashRegion::AddressOfSector(uint32_t sectorIndex) const
    {
        return master.AddressOfSector(sectorIndex + startSector) - masterAddressOfStartSector;
    }

    void SynchronousFlashRegion::WriteBuffer(infra::ConstByteRange buffer, uint32_t address)
    {
        assert(SectorOfAddress(address) <= numberOfSectors && SectorOfAddress(address + buffer.size()) <= numberOfSectors);
        master.WriteBuffer(buffer, address + masterAddressOfStartSector);
    }

    void SynchronousFlashRegion::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        assert(SectorOfAddress(address) <= numberOfSectors && SectorOfAddress(address + buffer.size()) <= numberOfSectors);
        master.ReadBuffer(buffer, address + masterAddressOfStartSector);
    }

    void SynchronousFlashRegion::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        assert(beginIndex <= numberOfSectors && endIndex <= numberOfSectors);
        master.EraseSectors(beginIndex + startSector, endIndex + startSector);
    }
}
