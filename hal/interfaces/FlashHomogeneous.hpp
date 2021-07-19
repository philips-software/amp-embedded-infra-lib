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
}

#endif
