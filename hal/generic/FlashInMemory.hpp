#ifndef HAL_GENERIC_FLASH_IN_MEMORY_HPP
#define HAL_GENERIC_FLASH_IN_MEMORY_HPP

#include "hal/interfaces/Flash.hpp"
#include <cstdint>
#include <vector>

namespace hal
{
    template<class T>
    class FlashInMemoryBase;

    using FlashInMemory = FlashInMemoryBase<uint32_t>;
    using FlashInMemory64 = FlashInMemoryBase<uint64_t>;

    template<class T>
    class FlashInMemoryBase
        : public hal::FlashBase<T>
    {
    public:
        FlashInMemoryBase(T numberOfSectors, uint32_t sizeOfEachSector, uint8_t erasedValue = 0xff);

        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;

        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;
        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

        std::vector<std::vector<uint8_t>>& Sectors();
        const std::vector<std::vector<uint8_t>>& Sectors() const;

    private:
        std::vector<std::vector<uint8_t>> sectors;
        uint8_t erasedValue;
    };
}

#endif
