#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_ID_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_ID_HPP

#include "infra/util/ByteRange.hpp"

namespace hal
{
    class SynchronousFlashId
    {
    public:
        virtual void ReadFlashId(infra::ByteRange buffer) = 0;
    };
}

#endif
