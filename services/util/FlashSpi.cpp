#include "services/util/FlashSpi.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/Endian.hpp"

namespace services
{
    const std::array<uint8_t, 2> FlashSpi::commandPageProgram{ 0x02, 0x12 }; // 3-byte addressing, 4-byte addressing
    const std::array<uint8_t, 2> FlashSpi::commandReadData{ 0x03, 0x13 };
    const std::array<uint8_t, 2> FlashSpi::commandEraseSubSector{ 0x20, 0x21 };
    const std::array<uint8_t, 2> FlashSpi::commandEraseSector{ 0xd8, 0xdc };
    const uint8_t FlashSpi::commandReadStatusRegister = 0x05;
    const uint8_t FlashSpi::commandWriteEnable = 0x06;
    const uint8_t FlashSpi::commandEraseBulk = 0xc7;
    const uint8_t FlashSpi::commandReadId = 0x9f;

    FlashSpi::FlashSpi(hal::SpiMaster& spi, const Config& config, uint32_t timerId)
        : hal::FlashHomogeneous(config.nrOfSubSectors, config.sizeSubSector)
        , spi(spi)
        , config(config)
        , delayTimer(timerId)
    {}

    void FlashSpi::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->address = address;

        flashOperationClaimer.Claim([this, buffer, onDone]()
            {
                this->onDone = onDone;
                this->buffer = buffer;
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
                                        flashOperationClaimer.Release();
                                        this->onDone();
                                    });
                            });
                    });
            });
    }

    void FlashSpi::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        this->address = address;

        flashOperationClaimer.Claim([this, buffer, onDone]()
            {
                this->onDone = onDone;
                readBuffer = buffer;
                spi.SendData(InstructionAndAddress(commandReadData, this->address), hal::SpiAction::continueSession, [this]()
                    {
                        spi.ReceiveData(readBuffer, hal::SpiAction::stop, [this]()
                            {
                                flashOperationClaimer.Release();
                                this->onDone();
                            });
                    });
            });
    }

    void FlashSpi::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        sectorIndex = beginIndex;

        flashOperationClaimer.Claim([this, endIndex, onDone]()
            {
                this->onDone = onDone;
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
                                        flashOperationClaimer.Release();
                                        this->onDone();
                                    });
                            });
                    });
            });
    }

    void FlashSpi::ReadFlashId(infra::ByteRange buffer, infra::Function<void()> onDone)
    {
        flashIdClaimer.Claim([this, buffer, onDone]()
            {
                this->onDone = onDone;
                readBuffer = buffer;
                spi.SendData(infra::MakeByteRange(commandReadId), hal::SpiAction::continueSession, [this]()
                    {
                        spi.ReceiveData(readBuffer, hal::SpiAction::stop, [this]()
                            {
                                flashIdClaimer.Release();
                                this->onDone();
                            });
                    });
            });
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
        writeBuffer = infra::Head(buffer, config.sizePage - AddressOffsetInSector(address) % config.sizePage);
        buffer.pop_front(writeBuffer.size());

        auto instructionAndAddress = InstructionAndAddress(commandPageProgram, address);

        address += writeBuffer.size();

        spi.SendData(instructionAndAddress, hal::SpiAction::continueSession, [this]()
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
        else if (sectorIndex % (config.sizeSector / config.sizeSubSector) == 0 && sectorIndex + config.sizeSector / config.sizeSubSector <= endIndex)
        {
            SendEraseSector(sectorIndex);
            sectorIndex += config.sizeSector / config.sizeSubSector;
        }
        else
        {
            SendEraseSubSector(sectorIndex);
            ++sectorIndex;
        }
    }

    void FlashSpi::SendEraseSubSector(uint32_t subSectorIndex)
    {
        spi.SendData(InstructionAndAddress(commandEraseSubSector, AddressOfSector(subSectorIndex)), hal::SpiAction::stop, [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashSpi::SendEraseSector(uint32_t subSectorIndex)
    {
        spi.SendData(InstructionAndAddress(commandEraseSector, AddressOfSector(subSectorIndex)), hal::SpiAction::stop, [this]()
            {
                sequencer.Continue();
            });
    }

    void FlashSpi::SendEraseBulk()
    {
        spi.SendData(infra::MakeByteRange(commandEraseBulk), hal::SpiAction::stop, [this]()
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

    infra::ConstByteRange FlashSpi::InstructionAndAddress(const std::array<uint8_t, 2>& instruction, uint32_t address)
    {
        auto addressSize = config.extendedAddressing ? 4 : 3;
        address = infra::SwapEndian(address);
        auto addressRange = infra::MakeByteRange(address);
        auto addressRangeInBuffer = infra::ByteRange(instructionAndAddressBuffer.data() + 1, instructionAndAddressBuffer.data() + addressSize + 1);

        if (config.extendedAddressing)
        {
            instructionAndAddressBuffer[0] = instruction[1];
            std::copy(addressRange.begin(), addressRange.end(), addressRangeInBuffer.begin());
        }
        else
        {
            instructionAndAddressBuffer[0] = instruction[0];
            std::copy(addressRange.begin() + 1, addressRange.end(), addressRangeInBuffer.begin());
        }

        return infra::ByteRange(instructionAndAddressBuffer.data(), instructionAndAddressBuffer.data() + addressSize + 1);
    }

    infra::ClaimableResource& FlashSpi::Resource()
    {
        return resource;
    }

    hal::SpiMaster& FlashSpi::Spi()
    {
        return spi;
    }
}
