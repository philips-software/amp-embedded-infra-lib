#include "infra/util/ReallyAssert.hpp"
#include "services/util/FlashRegion.hpp"

namespace services
{
    template<class T>
    FlashRegionBase<T>::FlashRegionBase(hal::FlashBase<T>& master, T startSector, T numberOfSectors)
        : master(master)
        , startSector(startSector)
        , numberOfSectors(numberOfSectors)
        , masterAddressOfStartSector(master.AddressOfSector(startSector))
    {
        really_assert(startSector + numberOfSectors <= master.NumberOfSectors());
    }

    template<class T>
    T FlashRegionBase<T>::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    template<class T>
    uint32_t FlashRegionBase<T>::SizeOfSector(T sectorIndex) const
    {
        return master.SizeOfSector(startSector + sectorIndex);
    }

    template<class T>
    T FlashRegionBase<T>::SectorOfAddress(T address) const
    {
        return master.SectorOfAddress(masterAddressOfStartSector + address) - startSector;
    }

    template<class T>
    T FlashRegionBase<T>::AddressOfSector(T sectorIndex) const
    {
        return master.AddressOfSector(sectorIndex + startSector) - masterAddressOfStartSector;
    }

    template<class T>
    void FlashRegionBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
    {
        really_assert(SectorOfAddress(address) <= numberOfSectors && SectorOfAddress(address + buffer.size()) <= numberOfSectors);
        master.WriteBuffer(buffer, address + masterAddressOfStartSector, onDone);
    }

    template<class T>
    void FlashRegionBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
    {
        really_assert(SectorOfAddress(address) <= numberOfSectors && SectorOfAddress(address + buffer.size()) <= numberOfSectors);
        master.ReadBuffer(buffer, address + masterAddressOfStartSector, onDone);
    }

    template<class T>
    void FlashRegionBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
    {
        really_assert(beginIndex <= numberOfSectors && endIndex <= numberOfSectors);
        master.EraseSectors(beginIndex + startSector, endIndex + startSector, onDone);
    }

    template class FlashRegionBase<uint32_t>;
    template class FlashRegionBase<uint64_t>;
}
