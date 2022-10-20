#ifndef HAL_INTERFACE_FLASH_HPP
#define HAL_INTERFACE_FLASH_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include <cstdint>

namespace hal
{
    template<class T>
    class FlashBase;

    using Flash = FlashBase<uint32_t>;
    using Flash64 = FlashBase<uint64_t>;

    template<class T>
    class FlashBase
    {
    protected:
        FlashBase() = default;
        FlashBase(const FlashBase& other) = delete;
        FlashBase& operator=(const FlashBase& other) = delete;
        ~FlashBase() = default;

    public:
        virtual T NumberOfSectors() const = 0;
        virtual uint32_t SizeOfSector(T sectorIndex) const = 0;
        T TotalSize() const;

        virtual T SectorOfAddress(T address) const = 0;
        virtual T AddressOfSector(T sectorIndex) const = 0;
        T StartOfSector(T address) const;
        T AddressOffsetInSector(T address) const;
        T StartOfNextSector(T address) const;
        T StartOfPreviousSector(T address) const;
        T StartOfNextSectorCyclical(T address) const;
        T StartOfPreviousSectorCyclical(T address) const;
        bool AtStartOfSector(T address) const;

        virtual void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) = 0;
        virtual void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) = 0;

        void EraseSector(T sectorIndex, infra::Function<void()> onDone);
        virtual void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) = 0; // Erases sectors in de range [beginIndex, endIndex)
        void EraseAll(infra::Function<void()> onDone);
    };
}

#endif
