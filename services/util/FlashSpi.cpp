#include "services/util/FlashSpi.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{

    const uint8_t FlashSpi::commandPageProgram = 0x02;
    const uint8_t FlashSpi::commandReadData = 0x03;
    const uint8_t FlashSpi::commandReadStatusRegister = 0x05;
    const uint8_t FlashSpi::commandWriteEnable = 0x06;
    const uint8_t FlashSpi::commandEraseSubSector = 0x20;
    const uint8_t FlashSpi::commandEraseSector = 0xd8;
    const uint8_t FlashSpi::commandEraseBulk = 0xc7;
    const uint8_t FlashSpi::commandReadId = 0x9f;

    FlashSpi::FlashSpi(hal::SpiMaster& spi, uint32_t numberOfSubSectors, uint32_t timerId, infra::Function<void()> onInitialized)
        : hal::FlashHomogeneous(numberOfSubSectors, sizeSubSector)
        , spi(spi)
        , delayTimer(timerId)
    {
        onInitialized();
    }

    void FlashSpi::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        this->buffer = buffer;
        this->address = address;
        sequencer.Load([this]()
            {
                sequencer.While([this]()
                    {
                        return !this->buffer.empty();
                    });
                sequencer.Step([this]()
                    {
                        WriteEnable();
                    });
                sequencer.Step([this]()
                    {
                        PageProgram();
                    });
                HoldWhileWriteInProgress();
                sequencer.EndWhile();
                sequencer.Execute([this]()
                    {
                        infra::EventDispatcher::Instance().Schedule([this]()
                            {
                                this->onDone();
                            });
                    });
            });
    }

    void FlashSpi::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        readBuffer = buffer;
        this->onDone = onDone;
        instructionAndAddress.instruction = commandReadData;
        instructionAndAddress.address = ConvertAddress(address);

        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SpiAction::continueSession, [this]()
            {
                spi.ReceiveData(readBuffer, hal::SpiAction::stop, [this]()
                    {
                        this->onDone();
                    });
            });
    }

    void FlashSpi::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        sectorIndex = beginIndex;
        sequencer.Load([this, endIndex]()
            {
                sequencer.While([this, endIndex]()
                    {
                        return sectorIndex != endIndex;
                    });
                sequencer.Step([this]()
                    {
                        WriteEnable();
                    });
                sequencer.Step([this, endIndex]()
                    {
                        EraseSomeSectors(endIndex);
                    });
                HoldWhileWriteInProgress();
                sequencer.EndWhile();
                sequencer.Execute([this]()
                    {
                        infra::EventDispatcher::Instance().Schedule([this]()
                            {
                                this->onDone();
                            });
                    });
            });
    }

    void FlashSpi::ReadFlashId(infra::ByteRange buffer, infra::Function<void()> onDone)
    {
        this->onDone = onDone;
        readBuffer = buffer;

        spi.SendData(infra::MakeByteRange(commandReadId), hal::SpiAction::continueSession, [this]()
            {
                spi.ReceiveData(readBuffer, hal::SpiAction::stop, [this]()
                    {
                        this->onDone();
                    });
            });
    }

    std::array<uint8_t, 3> FlashSpi::ConvertAddress(uint32_t address) const
    {
        uint32_t linearAddress = address;
        std::array<uint8_t, 3> result;
        result[0] = static_cast<uint8_t>(linearAddress >> 16);
        result[1] = static_cast<uint8_t>(linearAddress >> 8);
        result[2] = static_cast<uint8_t>(linearAddress);
        return result;
    }

    void FlashSpi::WriteEnable()
    {
        static const uint8_t command = commandWriteEnable;

        spi.SendData(infra::MakeByteRange(command), hal::SpiAction::stop, [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashSpi::PageProgram()
    {
        writeBuffer = infra::Head(buffer, sizePage - AddressOffsetInSector(address) % sizePage);
        buffer.pop_front(writeBuffer.size());

        instructionAndAddress.instruction = commandPageProgram;
        instructionAndAddress.address = ConvertAddress(address);

        address += writeBuffer.size();

        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SpiAction::continueSession, [this]()
            {
                spi.SendData(writeBuffer, hal::SpiAction::stop, [this]()
                    {
                        sequencer.Continue();
                    });
            });
    }

    void FlashSpi::EraseSomeSectors(uint32_t endIndex)
    {
        if (sectorIndex == 0 && endIndex == NumberOfSectors())
        {
            SendEraseBulk();
            sectorIndex += NumberOfSectors();
        }
        else if (sectorIndex % (sizeSector / sizeSubSector) == 0 && sectorIndex + sizeSector / sizeSubSector <= endIndex)
        {
            SendEraseSector(sectorIndex);
            sectorIndex += sizeSector / sizeSubSector;
        }
        else
        {
            SendEraseSubSector(sectorIndex);
            ++sectorIndex;
        }
    }

    void FlashSpi::SendEraseSubSector(uint32_t subSectorIndex)
    {
        instructionAndAddress.instruction = commandEraseSubSector;
        instructionAndAddress.address = ConvertAddress(AddressOfSector(subSectorIndex));

        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SpiAction::stop, [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashSpi::SendEraseSector(uint32_t subSectorIndex)
    {
        instructionAndAddress.instruction = commandEraseSector;
        instructionAndAddress.address = ConvertAddress(AddressOfSector(subSectorIndex));

        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SpiAction::stop, [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashSpi::SendEraseBulk()
    {
        instructionAndAddress.instruction = commandEraseBulk;

        spi.SendData(infra::MakeByteRange(instructionAndAddress.instruction), hal::SpiAction::stop, [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashSpi::HoldWhileWriteInProgress()
    {
        sequencer.Step([this]()
            {
                ReadStatusRegister();
            });
        sequencer.While([this]()
            {
                return (statusRegister & 1) == 1;
            });
        sequencer.Step([this]()
            {
                delayTimer.Start(std::chrono::milliseconds(1), [this]()
                    {
                        sequencer.Continue();
                    });
            });
        sequencer.Step([this]()
            {
                ReadStatusRegister();
            });
        sequencer.EndWhile();
    }

    void FlashSpi::ReadStatusRegister()
    {
        static const uint8_t instruction = commandReadStatusRegister;

        spi.SendData(infra::MakeByteRange(instruction), hal::SpiAction::continueSession, [this]()
            {
                spi.ReceiveData(infra::MakeByteRange(statusRegister), hal::SpiAction::stop, [this]()
                    {
                        sequencer.Continue();
                    });
            });
    }
}
