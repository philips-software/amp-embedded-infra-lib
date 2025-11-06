#ifndef SERVICES_SLEEP_AFTER_OPERATION_FLASH_DECORATOR
#define SERVICES_SLEEP_AFTER_OPERATION_FLASH_DECORATOR

#include "hal/interfaces/Sleepable.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "services/util/FlashDelegate.hpp"
#include <chrono>
#include <variant>

namespace services
{
    template<class T>
    class SleepOnInactivityFlashDecoratorBase;

    using SleepOnInactivityFlashDecorator = SleepOnInactivityFlashDecoratorBase<uint32_t>;
    using SleepOnInactivityFlashDecorator64 = SleepOnInactivityFlashDecoratorBase<uint64_t>;

    template<typename T>
    class SleepOnInactivityFlashDecoratorBase
        : public FlashDelegateBase<T>
    {
    public:
        SleepOnInactivityFlashDecoratorBase(hal::FlashBase<T>& flash, hal::Sleepable& sleepable, infra::Duration inactivityTimeout = std::chrono::milliseconds(100));

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;

        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

    private:
        void ScheduleSleep();
        void EnsureAwakeAndExecute(const infra::Function<void()>& operation);

        hal::Sleepable& sleepable;

        struct WriteBufferContext
        {
            infra::ConstByteRange buffer;
            T address;
        };

        struct ReadBufferContext
        {
            infra::ByteRange buffer;
            T address;
        };

        struct EraseSectorsContext
        {
            T beginIndex;
            T endIndex;
        };

        std::variant<WriteBufferContext, ReadBufferContext, EraseSectorsContext> context;
        infra::AutoResetFunction<void()> onDone;
        infra::Duration inactivityTimeout;
        infra::TimerSingleShot inactivityTimer;
    };
}

#endif
