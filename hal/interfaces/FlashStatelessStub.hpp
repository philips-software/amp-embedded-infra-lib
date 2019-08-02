#ifndef HAL_INTERFACE_FLASH_STATELESS_STUB_HPP
#define HAL_INTERFACE_FLASH_STATELESS_STUB_HPP

#include "hal/interfaces/FlashHomogeneous.hpp"

namespace hal
{
    class FlashStatelessStub
        : public hal::FlashHomogeneous
    {
    public:
        FlashStatelessStub(uint32_t numberOfSectors, uint32_t sizeOfEachSector);

        virtual void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;
    };
}

#endif
