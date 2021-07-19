#include "hal/interfaces/FlashHomogeneous.hpp"

namespace hal
{
    template<class T>
    FlashHomogeneousBase<T>::FlashHomogeneousBase(T numberOfSectors, uint32_t sizeOfEachSector)
        : numberOfSectors(numberOfSectors)
        , sizeOfEachSector(sizeOfEachSector)
    {}

    template<class T>
    T FlashHomogeneousBase<T>::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    template<class T>
    uint32_t FlashHomogeneousBase<T>::SizeOfSector(T sectorIndex) const
    {
        return sizeOfEachSector;
    }

    template<class T>
    T FlashHomogeneousBase<T>::SectorOfAddress(T address) const
    {
        return address / sizeOfEachSector;
    }

    template<class T>
    T FlashHomogeneousBase<T>::AddressOfSector(T sectorIndex) const
    {
        return sectorIndex * sizeOfEachSector;
    }

    template class FlashHomogeneousBase<uint32_t>;
    template class FlashHomogeneousBase<uint64_t>;
}
