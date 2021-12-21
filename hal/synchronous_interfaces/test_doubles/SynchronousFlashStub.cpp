#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"

namespace hal
{

    SynchronousFlashStub::SynchronousFlashStub(uint32_t numberOfSectors, uint32_t sizeOfEachSector)
        : sectors(numberOfSectors, std::vector<uint8_t>(sizeOfEachSector, 0xff))
    {}

    uint32_t SynchronousFlashStub::NumberOfSectors() const
    {
        return sectors.size();
    }

    uint32_t SynchronousFlashStub::SizeOfSector(uint32_t sectorIndex) const
    {
        return sectors[sectorIndex].size();
    }

    uint32_t SynchronousFlashStub::SectorOfAddress(uint32_t address) const
    {
        for (std::size_t index = 0; index != sectors.size(); ++index)
        {
            if (address < sectors[index].size())
                return index;

            address -= sectors[index].size();
        }

        assert(address == 0);
        return sectors.size();
    }

    uint32_t SynchronousFlashStub::AddressOfSector(uint32_t sectorIndex) const
    {
        assert(sectorIndex <= sectors.size());

        uint32_t result = 0;

        for (uint32_t index = 0; index != sectorIndex; ++index)
            result += sectors[index].size();

        return result;
    }

    void SynchronousFlashStub::WriteBuffer(infra::ConstByteRange buffer, uint32_t address)
    {
        if (stopAfterWriteSteps)
        {
            if (*stopAfterWriteSteps > 0)
                --*stopAfterWriteSteps;
            else
                return;
        }

        WriteBufferImpl(buffer, address);
    }

    void SynchronousFlashStub::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        while (!buffer.empty())
            ReadBufferPart(buffer, address);
    }

    void SynchronousFlashStub::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        for (uint32_t sectorIndex = beginIndex; sectorIndex != endIndex; ++sectorIndex)
            std::fill(sectors[sectorIndex].begin(), sectors[sectorIndex].end(), 0xff);
    }

    void SynchronousFlashStub::WriteBufferImpl(infra::ConstByteRange buffer, uint32_t address)
    {
        while (!buffer.empty())
        {
            std::size_t size = std::min<uint32_t>(buffer.size(), SizeOfSector(SectorOfAddress(address)) - AddressOffsetInSector(address));

            ApplyBuffer(buffer, address, size);

            buffer.pop_front(size);
            address = StartOfNextSector(address);
        }
    }

    void SynchronousFlashStub::ApplyBuffer(infra::ConstByteRange buffer, uint32_t address, uint32_t size)
    {
        for (std::size_t i = 0; i != size; ++i)
            sectors[SectorOfAddress(address)][AddressOffsetInSector(address) + i] &= buffer[i];
    }

    void SynchronousFlashStub::ReadBufferPart(infra::ByteRange& buffer, uint32_t& address)
    {
        std::size_t size = std::min<uint32_t>(buffer.size(), SizeOfSector(SectorOfAddress(address)) - AddressOffsetInSector(address));
        std::copy(sectors[SectorOfAddress(address)].begin() + AddressOffsetInSector(address), sectors[SectorOfAddress(address)].begin() + AddressOffsetInSector(address) + size, buffer.begin());

        buffer.pop_front(size);
        address = StartOfNextSector(address);
    }
}
