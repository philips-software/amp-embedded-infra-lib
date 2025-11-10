#include "services/util/FlashMultipleAccess.hpp"

namespace services
{
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
        claimer.Claim([this, buffer, address]()
            {
                master.WriteBuffer(buffer, address, [this]()
                    {
                        claimer.Release();
                        this->onDone();
                    });
            });
    }

    void FlashMultipleAccess::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        claimer.Claim([this, buffer, address]()
            {
                master.ReadBuffer(buffer, address, [this]()
                    {
                        claimer.Release();
                        this->onDone();
                    });
            });
    }

    void FlashMultipleAccess::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        claimer.Claim([this, beginIndex, endIndex]()
            {
                master.EraseSectors(beginIndex, endIndex, [this]()
                    {
                        claimer.Release();
                        this->onDone();
                    });
            });
    }
}
