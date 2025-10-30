#include "services/util/SleepAfterOperationFlashDecorator.hpp"
#include <cstdint>

namespace services
{
    template<typename T>
    SleepAfterOperationFlashDecoratorBase<T>::SleepAfterOperationFlashDecoratorBase(hal::FlashBase<T>& flash, services::Sleepable& sleepable)
        : flash(flash)
        , sleepable(sleepable)
    {}

    template<typename T>
    T SleepAfterOperationFlashDecoratorBase<T>::NumberOfSectors() const
    {
        return flash.NumberOfSectors();
    }

    template<typename T>
    uint32_t SleepAfterOperationFlashDecoratorBase<T>::SizeOfSector(T sectorIndex) const
    {
        return uint32_t();
    }

    template<typename T>
    T SleepAfterOperationFlashDecoratorBase<T>::SectorOfAddress(T address) const
    {
        return T();
    }

    template<typename T>
    T SleepAfterOperationFlashDecoratorBase<T>::AddressOfSector(T sectorIndex) const
    {
        return T();
    }

    template<typename T>
    void SleepAfterOperationFlashDecoratorBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
    {}

    template<typename T>
    void SleepAfterOperationFlashDecoratorBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
    {}

    template<typename T>
    void SleepAfterOperationFlashDecoratorBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
    {}

    template class SleepAfterOperationFlashDecoratorBase<uint32_t>;
    template class SleepAfterOperationFlashDecoratorBase<uint64_t>;
}
