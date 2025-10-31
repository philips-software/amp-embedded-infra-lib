#include "hal/interfaces/Sleepable.hpp"
#include "services/util/SleepOnInactivityFlashDecorator.hpp"
#include <cstdint>

namespace services
{
    template<typename T>
    SleepOnInactivityFlashDecoratorBase<T>::SleepOnInactivityFlashDecoratorBase(hal::FlashBase<T>& flash, hal::Sleepable& sleepable)
        : flash(flash)
        , sleepable(sleepable)
    {}

    template<typename T>
    T SleepOnInactivityFlashDecoratorBase<T>::NumberOfSectors() const
    {
        return flash.NumberOfSectors();
    }

    template<typename T>
    uint32_t SleepOnInactivityFlashDecoratorBase<T>::SizeOfSector(T sectorIndex) const
    {
        return flash.SizeOfSector(sectorIndex);
    }

    template<typename T>
    T SleepOnInactivityFlashDecoratorBase<T>::SectorOfAddress(T address) const
    {
        return flash.SectorOfAddress(address);
    }

    template<typename T>
    T SleepOnInactivityFlashDecoratorBase<T>::AddressOfSector(T sectorIndex) const
    {
        return flash.AddressOfSector(sectorIndex);
    }

    template<typename T>
    void SleepOnInactivityFlashDecoratorBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
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
    void SleepOnInactivityFlashDecoratorBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
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
    void SleepOnInactivityFlashDecoratorBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
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

    template class SleepOnInactivityFlashDecoratorBase<uint32_t>;
    template class SleepOnInactivityFlashDecoratorBase<uint64_t>;
}
