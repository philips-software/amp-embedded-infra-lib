#include "hal/generic/SynchronousFlashInMemory.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <algorithm>

namespace hal
{
    SynchronousFlashInMemory::SynchronousFlashInMemory(uint32_t numberOfSectors, uint32_t sizeOfEachSector, uint8_t erasedValue)
        : sectors(numberOfSectors, std::vector<uint8_t>(sizeOfEachSector, erasedValue))
        , erasedValue(erasedValue)
    {}

    uint32_t SynchronousFlashInMemory::NumberOfSectors() const
    {
        return sectors.size();
    }

    uint32_t SynchronousFlashInMemory::SizeOfSector(uint32_t sectorIndex) const
    {
        return sectors[sectorIndex].size();
    }

    uint32_t SynchronousFlashInMemory::SectorOfAddress(uint32_t address) const
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

    uint32_t SynchronousFlashInMemory::AddressOfSector(uint32_t sectorIndex) const
    {
        assert(sectorIndex <= sectors.size());

        uint32_t result = 0;

        for (uint32_t index = 0; index != sectorIndex; ++index)
            result += sectors[index].size();

        return result;
    }

    void SynchronousFlashInMemory::WriteBuffer(infra::ConstByteRange buffer, uint32_t address)
    {
        really_assert(IsInRange(address, buffer.size()));

        while (!buffer.empty())
        {
            auto destination = SectorRangeAt(address, buffer.size());
            std::transform(destination.begin(), destination.end(), buffer.begin(), destination.begin(), [](uint8_t current, uint8_t written)
                {
                    return static_cast<uint8_t>(current & written);
                });

            buffer.pop_front(destination.size());
            address += destination.size();
        }
    }

    void SynchronousFlashInMemory::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        really_assert(IsInRange(address, buffer.size()));

        while (!buffer.empty())
        {
            auto source = SectorRangeAt(address, buffer.size());
            std::copy(source.begin(), source.end(), buffer.begin());

            buffer.pop_front(source.size());
            address += source.size();
        }
    }

    void SynchronousFlashInMemory::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        really_assert(beginIndex <= endIndex && endIndex <= sectors.size());

        for (uint32_t sectorIndex = beginIndex; sectorIndex != endIndex; ++sectorIndex)
            std::fill(sectors[sectorIndex].begin(), sectors[sectorIndex].end(), erasedValue);
    }

    infra::ConstByteRange SynchronousFlashInMemory::MemoryMappedRange() const
    {
        memoryMappedSnapshot.resize(AddressOfSector(NumberOfSectors()));

        auto destination = memoryMappedSnapshot.begin();
        for (const auto& sector : sectors)
            destination = std::copy(sector.begin(), sector.end(), destination);

        return infra::MakeRange(memoryMappedSnapshot);
    }

    std::vector<std::vector<uint8_t>>& SynchronousFlashInMemory::Sectors()
    {
        return sectors;
    }

    const std::vector<std::vector<uint8_t>>& SynchronousFlashInMemory::Sectors() const
    {
        return sectors;
    }

    std::vector<uint8_t> SynchronousFlashInMemory::Contents() const
    {
        std::vector<uint8_t> result;
        result.reserve(AddressOfSector(NumberOfSectors()));

        for (const auto& sector : sectors)
            result.insert(result.end(), sector.begin(), sector.end());

        return result;
    }

    bool SynchronousFlashInMemory::IsInRange(uint32_t address, std::size_t size) const
    {
        auto totalSize = AddressOfSector(NumberOfSectors());
        return address <= totalSize && size <= totalSize - address;
    }

    infra::ByteRange SynchronousFlashInMemory::SectorRangeAt(uint32_t address, std::size_t maxSize)
    {
        auto sectorIndex = SectorOfAddress(address);
        return infra::Head(infra::DiscardHead(infra::MakeRange(sectors[sectorIndex]), address - AddressOfSector(sectorIndex)), maxSize);
    }
}
