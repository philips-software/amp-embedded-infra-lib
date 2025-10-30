#ifndef SERVICES_SLEEP_AFTER_OPERATION_FLASH_DECORATOR
#define SERVICES_SLEEP_AFTER_OPERATION_FLASH_DECORATOR
#include "hal/interfaces/Flash.hpp"
#include "infra/util/Variant.hpp"

namespace services
{
    class Sleepable;

    template<class T>
    class SleepAfterOperationFlashDecoratorBase;

    using SleepAfterOperationFlashDecorator = SleepAfterOperationFlashDecoratorBase<uint32_t>;
    using SleepAfterOperationFlashDecorator64 = SleepAfterOperationFlashDecoratorBase<uint64_t>;

    template<typename T>
    class SleepAfterOperationFlashDecoratorBase : public hal::FlashBase<T>
    {
    public:
        explicit SleepAfterOperationFlashDecoratorBase(hal::FlashBase<T>& flash, services::Sleepable& sleepable);

        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;

        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;

        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

    private:
        hal::FlashBase<T>& flash;
        services::Sleepable& sleepable;

        infra::Variant<infra::ByteRange, infra::ConstByteRange> buffer;
        infra::Function<void()> onDone;
        T addressOrIndex;
    };
}
#endif
