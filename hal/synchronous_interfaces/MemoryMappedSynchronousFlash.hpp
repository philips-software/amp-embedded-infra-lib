#ifndef SYNCHRONOUS_HAL_MEMORY_MAPPED_SYNCHRONOUS_FLASH_HPP
#define SYNCHRONOUS_HAL_MEMORY_MAPPED_SYNCHRONOUS_FLASH_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include "infra/util/ByteRange.hpp"

namespace hal
{
    class MemoryMappedSynchronousFlash
        : public SynchronousFlash
    {
    protected:
        ~MemoryMappedSynchronousFlash() = default;

    public:
        // Returns the complete contents of the flash as one contiguous range of TotalSize() bytes
        virtual infra::ConstByteRange MemoryMappedRange() const = 0;
    };
}

#endif
