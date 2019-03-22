#ifndef HAL_INTERFACE_FLASH_HOMOGENEOUS_HPP
#define HAL_INTERFACE_FLASH_HOMOGENEOUS_HPP

#include "hal/interfaces/Flash.hpp"

namespace hal
{
    class FlashHomogeneous
        : public Flash
    {
    public:
        FlashHomogeneous(uint32_t numberOfSectors, uint32_t sizeOfEachSector);

    protected:
        ~FlashHomogeneous() = default;

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
