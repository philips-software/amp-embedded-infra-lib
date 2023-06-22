#ifndef SERVICES_FLASH_REGION_HPP
#define SERVICES_FLASH_REGION_HPP

#include "hal/interfaces/Flash.hpp"

namespace services
{
    template<class T>
    class FlashRegionBase;

    using FlashRegion = FlashRegionBase<uint32_t>;
    using FlashRegion64 = FlashRegionBase<uint64_t>;

    template<class T>
    class FlashRegionBase
        : public hal::FlashBase<T>
    {
    public:
        FlashRegionBase(hal::FlashBase<T>& master, T startSector, T numberOfSectors);

    public:
        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;

        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;
        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

    private:
        hal::FlashBase<T>& master;
        T startSector;
        T numberOfSectors;
        T masterAddressOfStartSector;
    };
}

#endif
