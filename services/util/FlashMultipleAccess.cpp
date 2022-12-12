#include "services/util/FlashMultipleAccess.hpp"

namespace services
{
    FlashMultipleAccessMaster::FlashMultipleAccessMaster(hal::Flash& master)
        : master(master)
    {}

    uint32_t FlashMultipleAccessMaster::NumberOfSectors() const
    {
        return master.NumberOfSectors();
    }

    uint32_t FlashMultipleAccessMaster::SizeOfSector(uint32_t sectorIndex) const
    {
        return master.SizeOfSector(sectorIndex);
    }

    uint32_t FlashMultipleAccessMaster::SectorOfAddress(uint32_t address) const
    {
        return master.SectorOfAddress(address);
    }

    uint32_t FlashMultipleAccessMaster::AddressOfSector(uint32_t sectorIndex) const
    {
        return master.AddressOfSector(sectorIndex);
    }

    void FlashMultipleAccessMaster::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        master.WriteBuffer(buffer, address, onDone);
    }

    void FlashMultipleAccessMaster::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        master.ReadBuffer(buffer, address, onDone);
    }

    void FlashMultipleAccessMaster::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        master.EraseSectors(beginIndex, endIndex, onDone);
    }

    FlashMultipleAccess::FlashMultipleAccess(FlashMultipleAccessMaster& master)
        : master(master)
        , claimer(master)
    {}

    uint32_t FlashMultipleAccess::NumberOfSectors() const
    {
        return master.NumberOfSectors();
    }

    uint32_t FlashMultipleAccess::SizeOfSector(uint32_t sectorIndex) const
    {
        return master.SizeOfSector(sectorIndex);
    }

    uint32_t FlashMultipleAccess::SectorOfAddress(uint32_t address) const
    {
        return master.SectorOfAddress(address);
    }

    uint32_t FlashMultipleAccess::AddressOfSector(uint32_t sectorIndex) const
    {
        return master.AddressOfSector(sectorIndex);
    }

    void FlashMultipleAccess::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        claimer.Claim([this, buffer, address, onDone]()
            { master.WriteBuffer(buffer, address, [this]()
                  { claimer.Release(); this->onDone(); }); });
    }

    void FlashMultipleAccess::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        claimer.Claim([this, buffer, address, onDone]()
            { master.ReadBuffer(buffer, address, [this]()
                  { claimer.Release(); this->onDone(); }); });
    }

    void FlashMultipleAccess::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        claimer.Claim([this, beginIndex, endIndex, onDone]()
            { master.EraseSectors(beginIndex, endIndex, [this]()
                  { claimer.Release(); this->onDone(); }); });
    }
}
