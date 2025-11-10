#include "services/util/SleepOnInactivityFlashDecorator.hpp"
#include "hal/interfaces/Sleepable.hpp"
#include "services/util/FlashDelegate.hpp"
#include <cstdint>

namespace services
{
    template<typename T>
    SleepOnInactivityFlashDecoratorBase<T>::SleepOnInactivityFlashDecoratorBase(hal::FlashBase<T>& flash, hal::Sleepable& sleepable, infra::Duration inactivityTimeout)
        : FlashDelegateBase<T>(flash)
        , sleepable(sleepable)
        , inactivityTimeout(inactivityTimeout)
    {
        ScheduleSleep();
    }

    template<typename T>
    void SleepOnInactivityFlashDecoratorBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
    {
        context = WriteBufferContext{ buffer, address };
        this->onDone = onDone;

        EnsureAwakeAndExecute([this]
            {
                auto& context = std::get<WriteBufferContext>(this->context);
                FlashDelegateBase<T>::WriteBuffer(context.buffer, context.address, [this]
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
                FlashDelegateBase<T>::ReadBuffer(context.buffer, context.address, [this]
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
                FlashDelegateBase<T>::EraseSectors(context.beginIndex, context.endIndex, [this]
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
