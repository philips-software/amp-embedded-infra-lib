#include "services/util/SleepAfterOperationFlashDecorator.hpp"
#include "hal/interfaces/Sleepable.hpp"
#include <cstdint>

namespace services
{
    template<typename T>
    SleepAfterOperationFlashDecoratorBase<T>::SleepAfterOperationFlashDecoratorBase(hal::FlashBase<T>& flash, hal::Sleepable& sleepable)
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
        return flash.SizeOfSector(sectorIndex);
    }

    template<typename T>
    T SleepAfterOperationFlashDecoratorBase<T>::SectorOfAddress(T address) const
    {
        return flash.SectorOfAddress(address);
    }

    template<typename T>
    T SleepAfterOperationFlashDecoratorBase<T>::AddressOfSector(T sectorIndex) const
    {
        return flash.AddressOfSector(sectorIndex);
    }

    template<typename T>
    void SleepAfterOperationFlashDecoratorBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
    {
        context = WriteBufferContext{ buffer, address };
        this->onDone = onDone;

        sleepable.Wake([this]
            {
                auto& context = this->context.template Get<WriteBufferContext>();
                flash.WriteBuffer(context.buffer, context.address, [this]
                    {
                        sleepable.Sleep(this->onDone);
                    });
            });
    }

    template<typename T>
    void SleepAfterOperationFlashDecoratorBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
    {
        context = ReadBufferContext{ buffer, address };
        this->onDone = onDone;

        sleepable.Wake([this]
            {
                auto& context = this->context.template Get<ReadBufferContext>();
                flash.ReadBuffer(context.buffer, context.address, [this]
                    {
                        sleepable.Sleep(this->onDone);
                    });
            });
    }

    template<typename T>
    void SleepAfterOperationFlashDecoratorBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
    {
        context = EraseSectorsContext{ beginIndex, endIndex };
        this->onDone = onDone;

        sleepable.Wake([this]
            {
                auto& context = this->context.template Get<EraseSectorsContext>();
                flash.EraseSectors(context.beginIndex, context.endIndex, [this]
                    {
                        sleepable.Sleep(this->onDone);
                    });
            });
    }

    template class SleepAfterOperationFlashDecoratorBase<uint32_t>;
    template class SleepAfterOperationFlashDecoratorBase<uint64_t>;
}
