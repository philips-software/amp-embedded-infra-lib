#include "hal/interfaces/FlashHeterogeneous.hpp"
#include <cstdint>

namespace hal
{
    template<class T>
    FlashHeterogeneousBase<T>::FlashHeterogeneousBase(infra::MemoryRange<T> sectors)
        : sectors(sectors)
    {}

    template<class T>
    T FlashHeterogeneousBase<T>::NumberOfSectors() const
    {
        return static_cast<T>(sectors.size());
    }

    template<class T>
    uint32_t FlashHeterogeneousBase<T>::SizeOfSector(T sectorIndex) const
    {
        return static_cast<uint32_t>(sectors[sectorIndex]);
    }

    template<class T>
    T FlashHeterogeneousBase<T>::SectorOfAddress(T address) const
    {
        T totalSize = 0;
        for (T sector = 0; sector != sectors.size(); ++sector)
        {
            totalSize += sectors[sector];
            if (address < totalSize)
                return sector;
        }

        assert(address == totalSize);
        return static_cast<T>(sectors.size() - 1);
    }

    template<class T>
    T FlashHeterogeneousBase<T>::AddressOfSector(T sectorIndex) const
    {
        assert(sectorIndex < sectors.size());

        T address = 0;

        for (T index = 0; index != sectorIndex; ++index)
            address += sectors[index];

        return address;
    }

    template class FlashHeterogeneousBase<uint32_t>;
    template class FlashHeterogeneousBase<uint64_t>;
}
