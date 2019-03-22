#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace hal
{
    FlashStub::FlashStub(uint32_t numberOfSectors, uint32_t sizeOfEachSector)
        : sectors(numberOfSectors, std::vector<uint8_t>(sizeOfEachSector, 0xff))
    {}

    uint32_t FlashStub::NumberOfSectors() const
    {
        return sectors.size();
    }

    uint32_t FlashStub::SizeOfSector(uint32_t sectorIndex) const
    {
        return sectors[sectorIndex].size();
    }

    uint32_t FlashStub::SectorOfAddress(uint32_t address) const
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

    uint32_t FlashStub::AddressOfSector(uint32_t sectorIndex) const
    {
        assert(sectorIndex <= sectors.size());

        uint32_t result = 0;

        for (uint32_t index = 0; index != sectorIndex; ++index)
            result += sectors[index].size();

        return result;
    }

    void FlashStub::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        infra::EventDispatcher::Instance().Schedule(onDone);

        if (stopAfterWriteSteps)
            if (*stopAfterWriteSteps > 0)
                --*stopAfterWriteSteps;
            else
                return;

        while (!buffer.empty())
        {
            std::size_t size = std::min<uint32_t>(buffer.size(), SizeOfSector(SectorOfAddress(address)) - AddressOffsetInSector(address));

            for (std::size_t i = 0; i != size; ++i)
                sectors[SectorOfAddress(address)][AddressOffsetInSector(address) + i] &= buffer[i];

            buffer.pop_front(size);
            address = StartOfNextSector(address);
        }
    }

    void FlashStub::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        while (!buffer.empty())
        {
            std::size_t size = std::min<uint32_t>(buffer.size(), SizeOfSector(SectorOfAddress(address)) - AddressOffsetInSector(address));
            std::copy(sectors[SectorOfAddress(address)].begin() + AddressOffsetInSector(address), sectors[SectorOfAddress(address)].begin() + AddressOffsetInSector(address) + size, buffer.begin());

            buffer.pop_front(size);
            address = StartOfNextSectorCyclical(address);
        }

        infra::EventDispatcher::Instance().Schedule(onDone);
    }

    void FlashStub::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        for (uint32_t sectorIndex = beginIndex; sectorIndex != endIndex; ++sectorIndex)
            std::fill(sectors[sectorIndex].begin(), sectors[sectorIndex].end(), 0xff);

        onEraseDone = onDone;
        if (!delaySignalEraseDone)
            infra::EventDispatcher::Instance().Schedule(onDone);
    }
}
