#ifndef HAL_STUB_FLASH_STUB_HPP
#define HAL_STUB_FLASH_STUB_HPP

#include "hal/generic/FlashInMemory.hpp"
#include "infra/event/EventDispatcher.hpp"
#include <optional>

namespace hal
{
    template<class T>
    class FlashStubBase
        : public FlashInMemoryBase<T>
    {
    public:
        using FlashInMemoryBase<T>::FlashInMemoryBase;

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override
        {
            if (stopAfterWriteSteps)
            {
                if (*stopAfterWriteSteps == 0)
                {
                    infra::EventDispatcher::Instance().Schedule(onDone);
                    return;
                }

                --*stopAfterWriteSteps;
            }

            FlashInMemoryBase<T>::WriteBuffer(buffer, address, onDone);
        }

        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override
        {
            onEraseDone = onDone;

            if (delaySignalEraseDone)
                FlashInMemoryBase<T>::EraseSectors(beginIndex, endIndex, [] {});
            else
                FlashInMemoryBase<T>::EraseSectors(beginIndex, endIndex, onDone);
        }

        std::vector<std::vector<uint8_t>>& sectors{ this->Sectors() };
        std::optional<uint8_t> stopAfterWriteSteps;

        infra::Function<void()> onEraseDone;
        bool delaySignalEraseDone = false;
    };

    using FlashStub = FlashStubBase<uint32_t>;
    using FlashStub64 = FlashStubBase<uint64_t>;
}

#endif
