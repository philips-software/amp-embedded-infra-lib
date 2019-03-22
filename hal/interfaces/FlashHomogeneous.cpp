#include "hal/interfaces/FlashHomogeneous.hpp"

namespace hal
{
    FlashHomogeneous::FlashHomogeneous(uint32_t numberOfSectors, uint32_t sizeOfEachSector)
        : numberOfSectors(numberOfSectors)
        , sizeOfEachSector(sizeOfEachSector)
    {}

    uint32_t FlashHomogeneous::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    uint32_t FlashHomogeneous::SizeOfSector(uint32_t sectorIndex) const
    {
        return sizeOfEachSector;
    }

    uint32_t FlashHomogeneous::SectorOfAddress(uint32_t address) const
    {
        return address / sizeOfEachSector;
    }

    uint32_t FlashHomogeneous::AddressOfSector(uint32_t sectorIndex) const
    {
        return sectorIndex * sizeOfEachSector;
    }
}
