#ifndef HAL_INTERFACE_FLASH_ID_HPP
#define HAL_INTERFACE_FLASH_ID_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"

namespace hal
{
    class FlashId
    {
    public:
        virtual void ReadFlashId(infra::ByteRange buffer, infra::Function<void()> onDone) = 0;
    };
}

#endif
