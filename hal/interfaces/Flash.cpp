#include "hal/interfaces/Flash.hpp"

namespace hal
{
    template<class T>
    T FlashBase<T>::TotalSize() const
    {
        return AddressOfSector(NumberOfSectors() - 1) + SizeOfSector(NumberOfSectors() - 1);
    }

    template<class T>
    T FlashBase<T>::StartOfSector(T address) const
    {
        return AddressOfSector(SectorOfAddress(address));
    }

    template<class T>
    T FlashBase<T>::AddressOffsetInSector(T address) const
    {
        return address - StartOfSector(address);
    }

    template<class T>
    T FlashBase<T>::StartOfNextSector(T address) const
    {
        return AddressOfSector(SectorOfAddress(address) + 1);
    }

    template<class T>
    T FlashBase<T>::StartOfPreviousSector(T address) const
    {
        return AddressOfSector(SectorOfAddress(address) - 1);
    }

    template<class T>
    T FlashBase<T>::StartOfNextSectorCyclical(T address) const
    {
        T sectorIndex = SectorOfAddress(address) + 1;
        if (sectorIndex == NumberOfSectors())
            sectorIndex = 0;
        return AddressOfSector(sectorIndex);
    }

    template<class T>
    T FlashBase<T>::StartOfPreviousSectorCyclical(T address) const
    {
        T sectorIndex = SectorOfAddress(address);
        if (sectorIndex == 0)
            sectorIndex = NumberOfSectors();
        return AddressOfSector(sectorIndex - 1);
    }

    template<class T>
    bool FlashBase<T>::AtStartOfSector(T address) const
    {
        return StartOfSector(address) == address;
    }

    template<class T>
    void FlashBase<T>::EraseSector(T sectorIndex, infra::Function<void()> onDone)
    {
        EraseSectors(sectorIndex, sectorIndex + 1, onDone);
    }

    template<class T>
    void FlashBase<T>::EraseAll(infra::Function<void()> onDone)
    {
        EraseSectors(0, NumberOfSectors(), onDone);
    }

    template class FlashBase<uint32_t>;
    template class FlashBase<uint64_t>;
}
