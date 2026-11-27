#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_STUB_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_STUB_HPP

#include "hal/generic/SynchronousFlashInMemory.hpp"
#include <optional>

namespace hal
{
    class SynchronousFlashStub
        : public SynchronousFlashInMemory
    {
    public:
        using SynchronousFlashInMemory::SynchronousFlashInMemory;

        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override
        {
            if (stopAfterWriteSteps)
            {
                if (*stopAfterWriteSteps == 0)
                    return;

                --*stopAfterWriteSteps;
            }

            SynchronousFlashInMemory::WriteBuffer(buffer, address);
        }

        std::vector<std::vector<uint8_t>>& sectors{ Sectors() };
        std::optional<uint8_t> stopAfterWriteSteps;
    };
}

#endif
