#include "hal/synchronous_interfaces/SynchronousFlashHomogeneous.hpp"

namespace hal
{
    SynchronousFlashHomogeneous::SynchronousFlashHomogeneous(uint32_t numberOfSectors, uint32_t sizeOfEachSector)
        : numberOfSectors(numberOfSectors)
        , sizeOfEachSector(sizeOfEachSector)
    {}

    uint32_t SynchronousFlashHomogeneous::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    uint32_t SynchronousFlashHomogeneous::SizeOfSector(uint32_t sectorIndex) const
    {
        return sizeOfEachSector;
    }

    uint32_t SynchronousFlashHomogeneous::SectorOfAddress(uint32_t address) const
    {
        return address / sizeOfEachSector;
    }

    uint32_t SynchronousFlashHomogeneous::AddressOfSector(uint32_t sectorIndex) const
    {
        return sectorIndex * sizeOfEachSector;
    }
}
