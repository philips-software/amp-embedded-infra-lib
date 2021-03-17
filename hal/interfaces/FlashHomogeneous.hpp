#ifndef HAL_INTERFACE_FLASH_HOMOGENEOUS_HPP
#define HAL_INTERFACE_FLASH_HOMOGENEOUS_HPP

#include "hal/interfaces/Flash.hpp"

namespace hal
{
    template<class T>
    class FlashHomogeneousBase;

    using FlashHomogeneous = FlashHomogeneousBase<uint32_t>;
    using FlashHomogeneous64 = FlashHomogeneousBase<uint64_t>;

    template<class T>
    class FlashHomogeneousBase
        : public FlashBase<T>
    {
    public:
        FlashHomogeneousBase(T numberOfSectors, uint32_t sizeOfEachSector);

    protected:
        ~FlashHomogeneousBase() = default;

    public:
        virtual T NumberOfSectors() const override;
        virtual uint32_t SizeOfSector(T sectorIndex) const override;

        virtual T SectorOfAddress(T address) const override;
        virtual T AddressOfSector(T sectorIndex) const override;

    private:
        T numberOfSectors;
        uint32_t sizeOfEachSector;
    };

#ifndef _MSC_VER
    extern template class FlashHomogeneousBase<uint32_t>;
    extern template class FlashHomogeneousBase<uint64_t>;
#endif

    ////    Implementation    ////

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
}

#endif
