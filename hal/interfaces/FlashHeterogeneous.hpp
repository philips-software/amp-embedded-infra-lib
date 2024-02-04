#ifndef HAL_INTERFACE_FLASH_HETEROGENEOUS_HPP
#define HAL_INTERFACE_FLASH_HETEROGENEOUS_HPP

#include "hal/interfaces/Flash.hpp"

namespace hal
{
    template<class T>
    class FlashHeterogeneousBase;

    using FlashHeterogeneous = FlashHeterogeneousBase<uint32_t>;
    using FlashHeterogeneous64 = FlashHeterogeneousBase<uint64_t>;

    template<class T>
    class FlashHeterogeneousBase
        : public FlashBase<T>
    {
    public:
        FlashHeterogeneousBase(infra::MemoryRange<T> sectors);

    protected:
        ~FlashHeterogeneousBase() = default;

    public:
        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;

        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;

    private:
        infra::MemoryRange<T> sectors;
    };
}

#endif
