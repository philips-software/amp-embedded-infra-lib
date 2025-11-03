#include "services/util/SleepOnInactivityFlashDecorator.hpp"
#include "hal/interfaces/Sleepable.hpp"
#include <cstdint>

namespace services
{
    template<typename T>
    SleepOnInactivityFlashDecoratorBase<T>::SleepOnInactivityFlashDecoratorBase(hal::FlashBase<T>& flash, hal::Sleepable& sleepable, infra::Duration inactivityTimeout)
        : flash(flash)
        , sleepable(sleepable)
        , inactivityTimeout(inactivityTimeout)
    {
        ScheduleSleep();
    }

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

        EnsureAwakeAndExecute([this]
            {
                auto& context = std::get<WriteBufferContext>(this->context);
                flash.WriteBuffer(context.buffer, context.address, [this]
                    {
                        ScheduleSleep();
                        this->onDone();
                    });
            });
    }

    template<typename T>
    void SleepOnInactivityFlashDecoratorBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
    {
        context = ReadBufferContext{ buffer, address };
        this->onDone = onDone;

        EnsureAwakeAndExecute([this]
            {
                auto& context = std::get<ReadBufferContext>(this->context);
                flash.ReadBuffer(context.buffer, context.address, [this]
                    {
                        ScheduleSleep();
                        this->onDone();
                    });
            });
    }

    template<typename T>
    void SleepOnInactivityFlashDecoratorBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
    {
        context = EraseSectorsContext{ beginIndex, endIndex };
        this->onDone = onDone;

        EnsureAwakeAndExecute([this]
            {
                auto& context = std::get<EraseSectorsContext>(this->context);
                flash.EraseSectors(context.beginIndex, context.endIndex, [this]
                    {
                        ScheduleSleep();
                        this->onDone();
                    });
            });
    }

    template<typename T>
    void SleepOnInactivityFlashDecoratorBase<T>::ScheduleSleep()
    {
        inactivityTimer.Start(inactivityTimeout, [this]
            {
                sleepable.Sleep(infra::emptyFunction);
            });
    }

    template<typename T>
    void SleepOnInactivityFlashDecoratorBase<T>::EnsureAwakeAndExecute(const infra::Function<void()>& operation)
    {
        auto armed = inactivityTimer.Armed();
        inactivityTimer.Cancel();
        if (armed)
            operation();
        else
            sleepable.Wake(operation);
    }

    template class SleepOnInactivityFlashDecoratorBase<uint32_t>;
    template class SleepOnInactivityFlashDecoratorBase<uint64_t>;
}
