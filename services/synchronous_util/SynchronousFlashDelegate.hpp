#ifndef SERVICES_SYNCHRONOUS_FLASH_DELEGATE_HPP
#define SERVICES_SYNCHRONOUS_FLASH_DELEGATE_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include <cstdint>

namespace upgrade::application
{
    class SynchronousFlashDelegateBase
        : public hal::SynchronousFlash
    {
    public:
        explicit SynchronousFlashDelegateBase(hal::SynchronousFlash& delegate);

        uint32_t NumberOfSectors() const override;
        uint32_t SizeOfSector(uint32_t sectorIndex) const override;
        uint32_t SectorOfAddress(uint32_t address) const override;
        uint32_t AddressOfSector(uint32_t sectorIndex) const override;
        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex) override;

    private:
        hal::SynchronousFlash& delegate_;
    };
}

#endif
