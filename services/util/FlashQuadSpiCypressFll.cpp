#include "services/util/FlashQuadSpiCypressFll.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    const uint8_t FlashQuadSpiCypressFll::commandPageProgram = 0x02;
    const uint8_t FlashQuadSpiCypressFll::commandReadData = 0xeb;
    const uint8_t FlashQuadSpiCypressFll::commandReadStatusRegister = 0x05;
    const uint8_t FlashQuadSpiCypressFll::commandWriteEnable = 0x06;
    const uint8_t FlashQuadSpiCypressFll::commandEraseSector = 0x20;
    const uint8_t FlashQuadSpiCypressFll::commandEraseHalfBlock = 0x52;
    const uint8_t FlashQuadSpiCypressFll::commandEraseBlock = 0xd8;
    const uint8_t FlashQuadSpiCypressFll::commandEraseChip = 0x60;
    const uint8_t FlashQuadSpiCypressFll::commandEnterQpi = 0x38;
    const uint8_t FlashQuadSpiCypressFll::commandExitQpi = 0xf5;
    const uint8_t FlashQuadSpiCypressFll::commandReadUniqueId = 0x4b;

    FlashQuadSpiCypressFll::FlashQuadSpiCypressFll(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors)
        : FlashQuadSpi(spi, numberOfSectors, commandPageProgram)
        , onInitialized(onInitialized)
    {
        sequencer.Load([this]()
            {
                sequencer.Step([this]()
                    {
                        initDelayTimer.Start(std::chrono::milliseconds(100), [this]()
                            {
                                sequencer.Continue();
                            });
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

    void FlashQuadSpiCypressFll::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        const hal::QuadSpi::Header header{ std::make_optional(commandReadData), hal::QuadSpi::AddressToVector(address << 8, 4), {}, 8 };
        spi.ReceiveData(header, buffer, hal::QuadSpi::Lines::QuadSpeed(), onDone);
    }

    void FlashQuadSpiCypressFll::SwitchToSingleSpeed(infra::Function<void()> onDone)
    {
        static const hal::QuadSpi::Header exitQpiHeader{ std::make_optional(commandExitQpi), {}, {}, 0 };
        spi.SendData(exitQpiHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), onDone);
    }

    void FlashQuadSpiCypressFll::SwitchToQuadSpeed()
    {
        static const hal::QuadSpi::Header enterQpiHeader{ std::make_optional(commandEnterQpi), {}, {}, 0 };
        spi.SendData(enterQpiHeader, {}, hal::QuadSpi::Lines::SingleSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::WriteEnable()
    {
        static const hal::QuadSpi::Header writeEnableHeader{ std::make_optional(commandWriteEnable), {}, {}, 0 };
        spi.SendData(writeEnableHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::EraseSomeSectors(uint32_t endIndex)
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
        else if (sectorIndex % (sizeHalfBlock / sizeSector) == 0 && sectorIndex + sizeHalfBlock / sizeSector <= endIndex)
        {
            SendEraseHalfBlock(sectorIndex);
            sectorIndex += sizeHalfBlock / sizeSector;
        }
        else
        {
            SendEraseSector(sectorIndex);
            ++sectorIndex;
        }
    }

    void FlashQuadSpiCypressFll::SendEraseSector(uint32_t sectorIndex)
    {
        hal::QuadSpi::Header eraseSectorHeader{ std::make_optional(commandEraseSector), ConvertAddress(AddressOfSector(sectorIndex)), {}, 0 };
        spi.SendData(eraseSectorHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::SendEraseHalfBlock(uint32_t sectorIndex)
    {
        hal::QuadSpi::Header eraseHalfBlockHeader{ std::make_optional(commandEraseHalfBlock), ConvertAddress(AddressOfSector(sectorIndex)), {}, 0 };
        spi.SendData(eraseHalfBlockHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::SendEraseBlock(uint32_t sectorIndex)
    {
        hal::QuadSpi::Header eraseBlockHeader{ std::make_optional(commandEraseBlock), ConvertAddress(AddressOfSector(sectorIndex)), {}, 0 };
        spi.SendData(eraseBlockHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::SendEraseChip()
    {
        static const hal::QuadSpi::Header eraseChipHeader{ std::make_optional(commandEraseChip), {}, {}, 0 };
        spi.SendData(eraseChipHeader, {}, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::HoldWhileWriteInProgress()
    {
        static const hal::QuadSpi::Header pollWriteInProgressHeader{ std::make_optional(commandReadStatusRegister), {}, {}, 0 };
        spi.PollStatus(pollWriteInProgressHeader, 1, 0, statusFlagWriteInProgress, hal::QuadSpi::Lines::QuadSpeed(), [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashQuadSpiCypressFll::ReadFlashId(infra::ByteRange buffer, infra::Function<void()> onDone)
    {
        static const hal::QuadSpi::Header readUniqueIdHeader{ std::make_optional(commandReadUniqueId), {}, {}, 16 };
        spi.ReceiveData(readUniqueIdHeader, buffer, hal::QuadSpi::Lines::QuadSpeed(), onDone);
    }
}
