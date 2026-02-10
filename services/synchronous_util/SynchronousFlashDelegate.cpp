#include "services/synchronous_util/SynchronousFlashDelegate.hpp"

namespace upgrade::application
{
    SynchronousFlashDelegateBase::SynchronousFlashDelegateBase(hal::SynchronousFlash& delegate)
        : delegate_(delegate)
    {}

    uint32_t SynchronousFlashDelegateBase::NumberOfSectors() const
    {
        return delegate_.NumberOfSectors();
    }

    uint32_t SynchronousFlashDelegateBase::SizeOfSector(uint32_t sectorIndex) const
    {
        return delegate_.SizeOfSector(sectorIndex);
    }

    uint32_t SynchronousFlashDelegateBase::SectorOfAddress(uint32_t address) const
    {
        return delegate_.SectorOfAddress(address);
    }

    uint32_t SynchronousFlashDelegateBase::AddressOfSector(uint32_t sectorIndex) const
    {
        return delegate_.AddressOfSector(sectorIndex);
    }

    void SynchronousFlashDelegateBase::WriteBuffer(infra::ConstByteRange buffer, uint32_t address)
    {
        delegate_.WriteBuffer(buffer, address);
    }

    void SynchronousFlashDelegateBase::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        delegate_.ReadBuffer(buffer, address);
    }

    void SynchronousFlashDelegateBase::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        delegate_.EraseSectors(beginIndex, endIndex);
    }
}
