#include "services/util/FlashQuadSpi.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    FlashQuadSpi::FlashQuadSpi(hal::QuadSpi& spi, uint32_t numberOfSectors, uint8_t commandPageProgram)
        : hal::FlashHomogeneous(numberOfSectors, sizeSector)
        , spi(spi)
        , commandPageProgram(commandPageProgram)
    {}

    void FlashQuadSpi::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        this->buffer = buffer;
        this->address = address;

        WriteBufferSequence();
    }

    void FlashQuadSpi::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        sectorIndex = beginIndex;
        sequencer.Load([this, endIndex]() {
            sequencer.While([this, endIndex]() { return sectorIndex != endIndex; });
            sequencer.Step([this]() { WriteEnable(); });
            sequencer.Step([this, endIndex]() { EraseSomeSectors(endIndex); });
            sequencer.Step([this]() { HoldWhileWriteInProgress(); });
            sequencer.EndWhile();
            sequencer.Execute([this]() { infra::EventDispatcher::Instance().Schedule([this]() { this->onDone(); }); });
        });
    }

    void FlashQuadSpi::WriteBufferSequence()
    {
        sequencer.Load([this]() {
            sequencer.While([this]() { return !this->buffer.empty(); });
            sequencer.Step([this]() { WriteEnable(); });
            sequencer.Step([this]() { PageProgram(); });
            sequencer.Step([this]() { HoldWhileWriteInProgress(); });
            sequencer.EndWhile();
            sequencer.Execute([this]() { infra::EventDispatcher::Instance().Schedule([this]() { this->onDone(); }); });
        });
    }

    void FlashQuadSpi::PageProgram()
    {
        hal::QuadSpi::Header pageProgramHeader{ infra::MakeOptional(commandPageProgram), ConvertAddress(address), {}, 0 };

        infra::ConstByteRange currentBuffer = infra::Head(buffer, sizePage - AddressOffsetInSector(address) % sizePage);
        buffer.pop_front(currentBuffer.size());
        address += currentBuffer.size();

        spi.SendData(pageProgramHeader, currentBuffer, hal::QuadSpi::Lines::QuadSpeed(), [this]() { sequencer.Continue(); });
    }

    infra::BoundedVector<uint8_t>::WithMaxSize<4> FlashQuadSpi::ConvertAddress(uint32_t address) const
    {
        return hal::QuadSpi::AddressToVector(address, 3);
    }
}
