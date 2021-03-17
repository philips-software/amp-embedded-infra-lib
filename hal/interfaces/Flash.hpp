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
        virtual void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) = 0;  // Erases sectors in de range [beginIndex, endIndex)
        void EraseAll(infra::Function<void()> onDone);
    };

#ifndef _MSC_VER
    extern template class FlashBase<uint32_t>;
    extern template class FlashBase<uint64_t>;
#endif

    ////    Implementation    ////

    template<class T>
    T FlashBase<T>::TotalSize() const
    {
        return AddressOfSector(NumberOfSectors() - 1) + SizeOfSector(NumberOfSectors() - 1);
    }

    template<class T>
    T FlashBase<T>::StartOfSector(T address) const
    {
        return AddressOfSector(SectorOfAddress(address));
    }

    template<class T>
    T FlashBase<T>::AddressOffsetInSector(T address) const
    {
        return address - StartOfSector(address);
    }

    template<class T>
    T FlashBase<T>::StartOfNextSector(T address) const
    {
        return AddressOfSector(SectorOfAddress(address) + 1);
    }

    template<class T>
    T FlashBase<T>::StartOfPreviousSector(T address) const
    {
        return AddressOfSector(SectorOfAddress(address) - 1);
    }

    template<class T>
    T FlashBase<T>::StartOfNextSectorCyclical(T address) const
    {
        T sectorIndex = SectorOfAddress(address) + 1;
        if (sectorIndex == NumberOfSectors())
            sectorIndex = 0;
        return AddressOfSector(sectorIndex);
    }

    template<class T>
    T FlashBase<T>::StartOfPreviousSectorCyclical(T address) const
    {
        T sectorIndex = SectorOfAddress(address);
        if (sectorIndex == 0)
            sectorIndex = NumberOfSectors();
        return AddressOfSector(sectorIndex - 1);
    }

    template<class T>
    bool FlashBase<T>::AtStartOfSector(T address) const
    {
        return StartOfSector(address) == address;
    }

    template<class T>
    void FlashBase<T>::EraseSector(T sectorIndex, infra::Function<void()> onDone)
    {
        EraseSectors(sectorIndex, sectorIndex + 1, onDone);
    }

    template<class T>
    void FlashBase<T>::EraseAll(infra::Function<void()> onDone)
    {
        EraseSectors(0, NumberOfSectors(), onDone);
    }
}

#endif
