#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace hal
{
    template<class T>
    FlashStubBase<T>::FlashStubBase(T numberOfSectors, uint32_t sizeOfEachSector)
        : sectors(static_cast<std::size_t>(numberOfSectors), std::vector<uint8_t>(sizeOfEachSector, 0xff))
    {}

    template<class T>
    T FlashStubBase<T>::NumberOfSectors() const
    {
        return sectors.size();
    }

    template<class T>
    uint32_t FlashStubBase<T>::SizeOfSector(T sectorIndex) const
    {
        return sectors[static_cast<std::size_t>(sectorIndex)].size();
    }

    template<class T>
    T FlashStubBase<T>::SectorOfAddress(T address) const
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

    template<class T>
    T FlashStubBase<T>::AddressOfSector(T sectorIndex) const
    {
        assert(sectorIndex <= sectors.size());

        uint32_t result = 0;

        for (uint32_t index = 0; index != sectorIndex; ++index)
            result += sectors[index].size();

        return result;
    }

    template<class T>
    void FlashStubBase<T>::WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone)
    {
        infra::EventDispatcher::Instance().Schedule(onDone);

        if (stopAfterWriteSteps)
            if (*stopAfterWriteSteps > 0)
                --*stopAfterWriteSteps;
            else
                return;

        while (!buffer.empty())
        {
            std::size_t size = std::min<uint32_t>(buffer.size(), SizeOfSector(SectorOfAddress(address)) - static_cast<std::size_t>(this->AddressOffsetInSector(address)));

            for (std::size_t i = 0; i != size; ++i)
                sectors[static_cast<std::size_t>(SectorOfAddress(address))][static_cast<std::size_t>(this->AddressOffsetInSector(address) + i)] &= buffer[i];

            buffer.pop_front(size);
            address = this->StartOfNextSector(address);
        }
    }

    template<class T>
    void FlashStubBase<T>::ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone)
    {
        while (!buffer.empty())
        {
            std::size_t size = std::min<uint32_t>(buffer.size(), SizeOfSector(SectorOfAddress(address)) - static_cast<std::size_t>(this->AddressOffsetInSector(address)));
            std::copy(sectors[static_cast<std::size_t>(SectorOfAddress(address))].begin() + static_cast<std::size_t>(this->AddressOffsetInSector(address)), sectors[static_cast<std::size_t>(SectorOfAddress(address))].begin() + static_cast<std::size_t>(this->AddressOffsetInSector(address)) + size, buffer.begin());

            buffer.pop_front(size);
            address = this->StartOfNextSectorCyclical(address);
        }

        infra::EventDispatcher::Instance().Schedule(onDone);
    }

    template<class T>
    void FlashStubBase<T>::EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone)
    {
        for (T sectorIndex = beginIndex; sectorIndex != endIndex; ++sectorIndex)
            std::fill(sectors[static_cast<std::size_t>(sectorIndex)].begin(), sectors[static_cast<std::size_t>(sectorIndex)].end(), 0xff);

        onEraseDone = onDone;
        if (!delaySignalEraseDone)
            infra::EventDispatcher::Instance().Schedule(onDone);
    }

    template class FlashStubBase<uint32_t>;
    template class FlashStubBase<uint64_t>;
}
