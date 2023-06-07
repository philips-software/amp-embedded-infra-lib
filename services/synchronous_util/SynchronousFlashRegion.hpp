#ifndef SERVICES_SYNCHRONOUS_FLASH_REGION_HPP
#define SERVICES_SYNCHRONOUS_FLASH_REGION_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"

namespace services
{
    class SynchronousFlashRegion
        : public hal::SynchronousFlash
    {
    public:
        SynchronousFlashRegion(hal::SynchronousFlash& master, uint32_t startSector, uint32_t numberOfSectors);

    public:
        uint32_t NumberOfSectors() const override;
        uint32_t SizeOfSector(uint32_t sectorIndex) const override;

        uint32_t SectorOfAddress(uint32_t address) const override;
        uint32_t AddressOfSector(uint32_t sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex) override;

    private:
        uint32_t ToMasterAddress(uint32_t address) const;

    private:
        hal::SynchronousFlash& master;
        uint32_t startSector;
        uint32_t numberOfSectors;
        uint32_t masterAddressOfStartSector;
    };
}

#endif
