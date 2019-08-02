#include "infra/event/EventDispatcher.hpp"
#include "hal/interfaces/FlashStatelessStub.hpp"

namespace hal
{
    FlashStatelessStub::FlashStatelessStub(uint32_t numberOfSectors, uint32_t sizeOfEachSector)
        : hal::FlashHomogeneous(numberOfSectors, sizeOfEachSector)
    {}

    void FlashStatelessStub::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        infra::EventDispatcher::Instance().Schedule(onDone);
    }

    void FlashStatelessStub::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        infra::EventDispatcher::Instance().Schedule(onDone);
    }

    void FlashStatelessStub::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        infra::EventDispatcher::Instance().Schedule(onDone);
    }
}
