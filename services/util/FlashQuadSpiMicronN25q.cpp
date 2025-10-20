#include "services/util/FlashQuadSpiMicronN25q.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    const uint8_t FlashQuadSpiMicronN25q::commandPageProgram = 0x32;
    const uint8_t FlashQuadSpiMicronN25q::commandReadData = 0x0B;
    const uint8_t FlashQuadSpiMicronN25q::commandReadStatusRegister = 0x05;
    const uint8_t FlashQuadSpiMicronN25q::commandReadFlagStatusRegister = 0x70;
    const uint8_t FlashQuadSpiMicronN25q::commandWriteEnable = 0x06;
    const uint8_t FlashQuadSpiMicronN25q::commandEraseSubSector = 0x20;
    const uint8_t FlashQuadSpiMicronN25q::commandEraseSector = 0xd8;
    const uint8_t FlashQuadSpiMicronN25q::commandEraseBulk = 0xc7;
    const uint8_t FlashQuadSpiMicronN25q::commandWriteEnhancedVolatileRegister = 0x61;
    const uint8_t FlashQuadSpiMicronN25q::commandReadEnhancedVolatileRegister = 0x65;

    const uint8_t FlashQuadSpiMicronN25q::volatileRegisterForQuadSpeed = 0x5f;

    FlashQuadSpiMicronN25q::FlashQuadSpiMicronN25q(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors)
        : FlashQuadSpi(spi, numberOfSectors, commandPageProgram)
        , onInitialized(onInitialized)
    {
        sequencer.Load([this]()
            {
                sequencer.Step([this]()
                    {
                        initDelayTimer.Start(std::chrono::milliseconds(1), [this]()
                            {
                                sequencer.Continue();
                            });
                    });
                sequencer.Step([this]()
                    {
                        WriteEnableSingleSpeed();
                    });
                sequencer.Step([this]()
                    {
                        SwitchToQuadSpeed();
                    });
                sequencer.Step([this]()
                    {
                        infra::EventDispatcher::Instance().Schedule([this]()
                            {
                                this->onInitialized();
                            });
                    });
            });
    }

    void FlashQuadSpiMicronN25q::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        const hal::QuadSpi::Header header{ std::make_optional(commandReadData), ConvertAddress(address), {}, 10 };

        spi.ReceiveData(header, buffer, hal::QuadSpi::Lines::QuadSpeed(), onDone);
    }

    void FlashQuadSpiMicronN25q::WriteEnableSingleSpeed()
    {
        static const hal::QuadSpi::Header writeEnableHeader{ std::make_optional(commandWriteEnable), {}, {}, 0 };
        spi.SendData(writeEnableHeader, {}, hal::QuadSpi::Lines::SingleSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiMicronN25q::SwitchToQuadSpeed()
    {
        static const hal::QuadSpi::Header writeVolatileRegisterHeader{ std::make_optional(commandWriteEnhancedVolatileRegister), {}, {}, 0 };
        spi.SendData(writeVolatileRegisterHeader, infra::MakeByteRange(volatileRegisterForQuadSpeed), hal::QuadSpi::Lines::SingleSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiMicronN25q::WriteEnable()
    {
        static const hal::QuadSpi::Header writeEnableHeader{ std::make_optional(commandWriteEnable), {}, {}, 0 };
        spi.SendData(writeEnableHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiMicronN25q::EraseSomeSectors(uint32_t endIndex)
    {
        if (sectorIndex == 0 && endIndex == NumberOfSectors())
        {
            SendEraseBulk();
            sectorIndex += NumberOfSectors();
        }
        else if (sectorIndex % (sizeBlock / sizeSector) == 0 && sectorIndex + sizeBlock / sizeSector <= endIndex)
        {
            SendEraseSector(sectorIndex);
            sectorIndex += sizeBlock / sizeSector;
        }
        else
        {
            SendEraseSubSector(sectorIndex);
            ++sectorIndex;
        }
    }

    void FlashQuadSpiMicronN25q::SendEraseSubSector(uint32_t subSectorIndex)
    {
        hal::QuadSpi::Header eraseSubSectorHeader{ std::make_optional(commandEraseSubSector), ConvertAddress(AddressOfSector(subSectorIndex)), {}, 0 };
        spi.SendData(eraseSubSectorHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiMicronN25q::SendEraseSector(uint32_t subSectorIndex)
    {
        hal::QuadSpi::Header eraseSectorHeader{ std::make_optional(commandEraseSector), ConvertAddress(AddressOfSector(subSectorIndex)), {}, 0 };
        spi.SendData(eraseSectorHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiMicronN25q::SendEraseBulk()
    {
        static const hal::QuadSpi::Header eraseBulkHeader{ std::make_optional(commandEraseBulk), {}, {}, 0 };
        spi.SendData(eraseBulkHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiMicronN25q::HoldWhileWriteInProgress()
    {
        static const hal::QuadSpi::Header pollWriteInProgressHeader{ std::make_optional(commandReadStatusRegister), {}, {}, 0 };
        spi.PollStatus(pollWriteInProgressHeader, 1, 0, statusFlagWriteInProgress, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }
}
