#include "services/util/FlashDelayedInit.hpp"

namespace services
{
    FlashDelayedInit::FlashDelayedInit(hal::Flash& master)
        : master(master)
    {}

    uint32_t FlashDelayedInit::NumberOfSectors() const
    {
        return master.NumberOfSectors();
    }

    uint32_t FlashDelayedInit::SizeOfSector(uint32_t sectorIndex) const
    {
        return master.SizeOfSector(sectorIndex);
    }

    uint32_t FlashDelayedInit::SectorOfAddress(uint32_t address) const
    {
        return master.SectorOfAddress(address);
    }

    uint32_t FlashDelayedInit::AddressOfSector(uint32_t sectorIndex) const
    {
        return master.AddressOfSector(sectorIndex);
    }

    void FlashDelayedInit::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        if (initialized)
            master.WriteBuffer(buffer, address, onDone);
        else
            onInitialized = [this, buffer, address, onDone]() {
                master.WriteBuffer(buffer, address, onDone);
            };
    }

    void FlashDelayedInit::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        if (initialized)
            master.ReadBuffer(buffer, address, onDone);
        else
            onInitialized = [this, buffer, address, onDone]() {
                master.ReadBuffer(buffer, address, onDone);
            };
    }

    void FlashDelayedInit::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        if (initialized)
            master.EraseSectors(beginIndex, endIndex, onDone);
        else
            onInitialized = [this, beginIndex, endIndex, onDone]() {
                master.EraseSectors(beginIndex, endIndex, onDone);
            };
    }

    void FlashDelayedInit::Initialized()
    {
        initialized = true;

        if (onInitialized)
            onInitialized();
    }
}
