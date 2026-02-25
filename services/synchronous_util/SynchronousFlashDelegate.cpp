#include "services/synchronous_util/SynchronousFlashDelegate.hpp"

namespace services
{
    SynchronousFlashDelegateBase::SynchronousFlashDelegateBase(hal::SynchronousFlash& parent)
        : delegate(parent)
    {}

    uint32_t SynchronousFlashDelegateBase::NumberOfSectors() const
    {
        return delegate.NumberOfSectors();
    }

    uint32_t SynchronousFlashDelegateBase::SizeOfSector(uint32_t sectorIndex) const
    {
        return delegate.SizeOfSector(sectorIndex);
    }

    uint32_t SynchronousFlashDelegateBase::SectorOfAddress(uint32_t address) const
    {
        return delegate.SectorOfAddress(address);
    }

    uint32_t SynchronousFlashDelegateBase::AddressOfSector(uint32_t sectorIndex) const
    {
        return delegate.AddressOfSector(sectorIndex);
    }

    void SynchronousFlashDelegateBase::WriteBuffer(infra::ConstByteRange buffer, uint32_t address)
    {
        delegate.WriteBuffer(buffer, address);
    }

    void SynchronousFlashDelegateBase::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        delegate.ReadBuffer(buffer, address);
    }

    void SynchronousFlashDelegateBase::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        delegate.EraseSectors(beginIndex, endIndex);
    }
}
