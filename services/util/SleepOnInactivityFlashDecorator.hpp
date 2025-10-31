#ifndef SERVICES_SLEEP_AFTER_OPERATION_FLASH_DECORATOR
#define SERVICES_SLEEP_AFTER_OPERATION_FLASH_DECORATOR

#include "hal/interfaces/Flash.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Variant.hpp"
#include <chrono>

namespace hal
{
    class Sleepable;
}

namespace services
{
    template<class T>
    class SleepOnInactivityFlashDecoratorBase;

    using SleepOnInactivityFlashDecorator = SleepOnInactivityFlashDecoratorBase<uint32_t>;
    using SleepOnInactivityFlashDecorator64 = SleepOnInactivityFlashDecoratorBase<uint64_t>;

    template<typename T>
    class SleepOnInactivityFlashDecoratorBase : public hal::FlashBase<T>
    {
    public:
        SleepOnInactivityFlashDecoratorBase(hal::FlashBase<T>& flash, hal::Sleepable& sleepable, infra::Duration inactivityTimeout = std::chrono::milliseconds(1));

        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;

        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;

        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

    private:
        void ScheduleSleep();
        void EnsureAwakeAndExecute(const infra::Function<void()>& operation);

        hal::FlashBase<T>& flash;
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

        infra::Variant<WriteBufferContext, ReadBufferContext, EraseSectorsContext> context;
        infra::Function<void()> onDone;
        infra::Duration inactivityTimeout;
        infra::TimerSingleShot inactivityTimer;
    };
}

#endif
