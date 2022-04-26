#include "services/util/FlashQuadSpiSingleSpeed.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    const uint8_t FlashQuadSpiSingleSpeed::commandPageProgram = 0x02;
    const uint8_t FlashQuadSpiSingleSpeed::commandReadData = 0x03;
    const uint8_t FlashQuadSpiSingleSpeed::commandReadStatusRegister = 0x05;
    const uint8_t FlashQuadSpiSingleSpeed::commandWriteEnable = 0x06;
    const uint8_t FlashQuadSpiSingleSpeed::commandEraseSector = 0x20;
    const uint8_t FlashQuadSpiSingleSpeed::commandEraseBlock = 0xd8;
    const uint8_t FlashQuadSpiSingleSpeed::commandEraseChip = 0x60;

    FlashQuadSpiSingleSpeed::FlashQuadSpiSingleSpeed(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors)
        : FlashQuadSpi(spi, numberOfSectors, commandPageProgram)
        , onInitialized(onInitialized)
        , initDelayTimer(std::chrono::milliseconds(100), [this]() { this->onInitialized(); })
    {}

    void FlashQuadSpiSingleSpeed::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        const hal::QuadSpi::Header header{ infra::MakeOptional(commandReadData), hal::QuadSpi::AddressToVector(address, 3), {}, 0 };

        spi.ReceiveData(header, buffer, hal::QuadSpi::Lines::SingleSpeed(), onDone);
    }

    void FlashQuadSpiSingleSpeed::PageProgram()
    {
        hal::QuadSpi::Header pageProgramHeader{ infra::MakeOptional(commandPageProgram), ConvertAddress(address), {}, 0 };

        infra::ConstByteRange currentBuffer = infra::Head(buffer, sizePage - AddressOffsetInSector(address) % sizePage);
        buffer.pop_front(currentBuffer.size());
        address += currentBuffer.size();

        spi.SendData(pageProgramHeader, currentBuffer, hal::QuadSpi::Lines::SingleSpeed(), [this]() { sequencer.Continue(); });
    }

    void FlashQuadSpiSingleSpeed::WriteEnable()
    {
        static const hal::QuadSpi::Header writeEnableHeader{ infra::MakeOptional(commandWriteEnable), {}, {}, 0 };
        spi.SendData(writeEnableHeader, {}, hal::QuadSpi::Lines::SingleSpeed(), [this]() { sequencer.Continue(); });
    }

    void FlashQuadSpiSingleSpeed::EraseSomeSectors(uint32_t endIndex)
    {
        if (sectorIndex == 0 && endIndex == NumberOfSectors())
        {
            SendEraseChip();
            sectorIndex += NumberOfSectors();
        }
        else if (sectorIndex % (sizeBlock / sizeSector) == 0 && sectorIndex + sizeBlock / sizeSector <= endIndex)
        {
            SendEraseBlock(sectorIndex);
            sectorIndex += sizeBlock / sizeSector;
        }
        else
        {
            SendEraseSector(sectorIndex);
            ++sectorIndex;
        }
    }

    void FlashQuadSpiSingleSpeed::SendEraseSector(uint32_t sectorIndex)
    {
        hal::QuadSpi::Header eraseSectorHeader{ infra::MakeOptional(commandEraseSector), ConvertAddress(AddressOfSector(sectorIndex)), {}, 0 };
        spi.SendData(eraseSectorHeader, {}, hal::QuadSpi::Lines::SingleSpeed(), [this]() { sequencer.Continue(); });
    }

    void FlashQuadSpiSingleSpeed::SendEraseBlock(uint32_t sectorIndex)
    {
        hal::QuadSpi::Header eraseBlockHeader{ infra::MakeOptional(commandEraseBlock), ConvertAddress(AddressOfSector(sectorIndex)), {}, 0 };
        spi.SendData(eraseBlockHeader, {}, hal::QuadSpi::Lines::SingleSpeed(), [this]() { sequencer.Continue(); });
    }

    void FlashQuadSpiSingleSpeed::SendEraseChip()
    {
        static const hal::QuadSpi::Header eraseChipHeader{ infra::MakeOptional(commandEraseChip), {}, {}, 0 };
        spi.SendData(eraseChipHeader, {}, hal::QuadSpi::Lines::SingleSpeed(), [this]() { sequencer.Continue(); });
    }

    void FlashQuadSpiSingleSpeed::HoldWhileWriteInProgress()
    {
        static const hal::QuadSpi::Header pollWriteInProgressHeader{ infra::MakeOptional(commandReadStatusRegister), {}, {}, 0 };
        spi.PollStatus(pollWriteInProgressHeader, 1, 0, statusFlagWriteInProgress, hal::QuadSpi::Lines::SingleSpeed(), [this]() { sequencer.Continue(); });
    }
}
