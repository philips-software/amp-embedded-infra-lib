#ifndef SERVICES_FLASH_DELEGATE_HPP
#define SERVICES_FLASH_DELEGATE_HPP

#include "hal/interfaces/Flash.hpp"

namespace services
{
    template<class T>
    class FlashDelegateBase;

    using FlashDelegate = FlashDelegateBase<uint32_t>;
    using FlashDelegate64 = FlashDelegateBase<uint64_t>;

    template<class T>
    class FlashDelegateBase
        : public hal::FlashBase<T>
    {
    public:
        explicit FlashDelegateBase(hal::FlashBase<T>& delegate);

        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;
        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;
        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;
        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

    private:
        hal::FlashBase<T>& delegate;
    };

    //// Implementation ////

    template<class T>
    FlashDelegateBase<T>::FlashDelegateBase(hal::FlashBase<T>& delegate)
        : delegate(delegate)
    {}

    template<class T>
    T FlashDelegateBase<T>::NumberOfSectors() const
    {
        return delegate.NumberOfSectors();
    }

    template<class T>
    uint32_t FlashDelegateBase<T>::SizeOfSector(T sectorIndex) const
    {
        return delegate.SizeOfSector(sectorIndex);
    }

    template<class T>
    T FlashDelegateBase<T>::SectorOfAddress(T address) const
    {
        return delegate.SectorOfAddress(address);
    }

    template<class T>
    T FlashDelegateBase<T>::AddressOfSector(T sectorIndex) const
    {
        return delegate.AddressOfSector(sectorIndex);
    }

    template<class T>
    void FlashDelegateBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
    {
        delegate.WriteBuffer(buffer, address, onDone);
    }

    template<class T>
    void FlashDelegateBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
    {
        delegate.ReadBuffer(buffer, address, onDone);
    }

    template<class T>
    void FlashDelegateBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
    {
        delegate.EraseSectors(beginIndex, endIndex, onDone);
    }
}

#endif
