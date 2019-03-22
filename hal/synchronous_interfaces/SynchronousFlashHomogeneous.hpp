#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_HOMOGENEOUS_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_HOMOGENEOUS_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"

namespace hal
{
    class SynchronousFlashHomogeneous
        : public hal::SynchronousFlash
    {
    public:
        SynchronousFlashHomogeneous(uint32_t numberOfSectors, uint32_t sizeOfEachSector);

    protected:
        ~SynchronousFlashHomogeneous() = default;

    public:
        virtual uint32_t NumberOfSectors() const override;
        virtual uint32_t SizeOfSector(uint32_t sectorIndex) const override;

        virtual uint32_t SectorOfAddress(uint32_t address) const override;
        virtual uint32_t AddressOfSector(uint32_t sectorIndex) const override;

    private:
        uint32_t numberOfSectors;
        uint32_t sizeOfEachSector;
    };
}

#endif
