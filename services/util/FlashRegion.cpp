#include "infra/util/ReallyAssert.hpp"
#include "services/util/FlashRegion.hpp"

namespace services
{
    FlashRegion::FlashRegion(hal::Flash& master, uint32_t startSector, uint32_t numberOfSectors)
        : master(master)
        , startSector(startSector)
        , numberOfSectors(numberOfSectors)
        , masterAddressOfStartSector(master.AddressOfSector(startSector))
    {
        really_assert(startSector + numberOfSectors <= master.NumberOfSectors());
    }

    uint32_t FlashRegion::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    uint32_t FlashRegion::SizeOfSector(uint32_t sectorIndex) const
    {
        return master.SizeOfSector(startSector + sectorIndex);
    }

    uint32_t FlashRegion::SectorOfAddress(uint32_t address) const
    {
        return master.SectorOfAddress(masterAddressOfStartSector + address) - startSector;
    }

    uint32_t FlashRegion::AddressOfSector(uint32_t sectorIndex) const
    {
        return master.AddressOfSector(sectorIndex + startSector) - masterAddressOfStartSector;
    }

    void FlashRegion::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        really_assert(SectorOfAddress(address) <= numberOfSectors && SectorOfAddress(address + buffer.size()) <= numberOfSectors);
        master.WriteBuffer(buffer, address + masterAddressOfStartSector, onDone);
    }

    void FlashRegion::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        really_assert(SectorOfAddress(address) <= numberOfSectors && SectorOfAddress(address + buffer.size()) <= numberOfSectors);
        master.ReadBuffer(buffer, address + masterAddressOfStartSector, onDone);
    }

    void FlashRegion::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        really_assert(beginIndex <= numberOfSectors && endIndex <= numberOfSectors);
        master.EraseSectors(beginIndex + startSector, endIndex + startSector, onDone);
    }
}
