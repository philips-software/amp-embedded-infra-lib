#include "services/synchronous_util/SynchronousFlashSpi.hpp"

namespace services
{
    const uint8_t SynchronousFlashSpi::commandPageProgram[2] = { 0x02, 0x12 }; // 3-byte addressing, 4-byte addressing
    const uint8_t SynchronousFlashSpi::commandReadData[2] = { 0x03, 0x13 };
    const uint8_t SynchronousFlashSpi::commandEraseSubSector[2] = { 0x20, 0x21 };
    const uint8_t SynchronousFlashSpi::commandEraseSector[2] = { 0xd8, 0xdc };
    const uint8_t SynchronousFlashSpi::commandReadStatusRegister = 0x05;
    const uint8_t SynchronousFlashSpi::commandWriteEnable = 0x06;
    const uint8_t SynchronousFlashSpi::commandEraseBulk = 0xc7;
    const uint8_t SynchronousFlashSpi::commandReadId = 0x9f;

    SynchronousFlashSpi::SynchronousFlashSpi(hal::SynchronousSpi& spi, const Config& config)
        : SynchronousFlashHomogeneous(config.numberOfSubSectors, sizeSubSector)
        , spi(spi)
        , config(config)
    {}

    void SynchronousFlashSpi::WriteBuffer(infra::ConstByteRange buffer_, uint32_t address_)
    {
        buffer = buffer_;
        address = address_;

        while (!buffer.empty())
        {
            WriteEnable();
            PageProgram();
            HoldWhileWriteInProgress();
        }
    }

    void SynchronousFlashSpi::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        infra::ByteRange instructionAndAddress = InstructionAndAddress(commandReadData, address);
        spi.SendData(instructionAndAddress, hal::SynchronousSpi::continueSession);
        spi.ReceiveData(buffer, hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        sectorIndex = beginIndex;
        while (sectorIndex != endIndex)
        {
            WriteEnable();
            EraseSomeSectors(endIndex);
            HoldWhileWriteInProgress();
        }
    }

    void SynchronousFlashSpi::ReadFlashId(infra::ByteRange buffer)
    {
        spi.SendData(infra::MakeByteRange(commandReadId), hal::SynchronousSpi::continueSession);
        spi.ReceiveData(buffer, hal::SynchronousSpi::stop);
    }

    infra::ByteRange SynchronousFlashSpi::InstructionAndAddress(const uint8_t instruction[], uint32_t address)
    {
        if (config.extendedAddressing)
        {
            instructionAndAddressBuffer[0] = instruction[1];
            instructionAndAddressBuffer[1] = static_cast<uint8_t>(address >> 24);
            instructionAndAddressBuffer[2] = static_cast<uint8_t>(address >> 16);
            instructionAndAddressBuffer[3] = static_cast<uint8_t>(address >> 8);
            instructionAndAddressBuffer[4] = static_cast<uint8_t>(address);
            return infra::ByteRange(instructionAndAddressBuffer.data(), instructionAndAddressBuffer.data() + 5);
        }
        else
        {
            instructionAndAddressBuffer[0] = instruction[0];
            instructionAndAddressBuffer[1] = static_cast<uint8_t>(address >> 16);
            instructionAndAddressBuffer[2] = static_cast<uint8_t>(address >> 8);
            instructionAndAddressBuffer[3] = static_cast<uint8_t>(address);
            return infra::ByteRange(instructionAndAddressBuffer.data(), instructionAndAddressBuffer.data() + 4);
        }
    }

    void SynchronousFlashSpi::WriteEnable()
    {
        static const uint8_t command = commandWriteEnable;
        spi.SendData(infra::MakeByteRange(command), hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::PageProgram()
    {
        infra::ConstByteRange currentBuffer = infra::Head(buffer, sizePage - AddressOffsetInSector(address) % sizePage);
        buffer.pop_front(currentBuffer.size());

        infra::ByteRange instructionAndAddress = InstructionAndAddress(commandPageProgram, address);
        spi.SendData(instructionAndAddress, hal::SynchronousSpi::continueSession);
        spi.SendData(currentBuffer, hal::SynchronousSpi::stop);

        address += currentBuffer.size();
    }

    void SynchronousFlashSpi::EraseSomeSectors(uint32_t endIndex)
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

    void SynchronousFlashSpi::SendEraseSubSector(uint32_t subSectorIndex)
    {
        infra::ByteRange instructionAndAddress = InstructionAndAddress(commandEraseSubSector, AddressOfSector(subSectorIndex));
        spi.SendData(instructionAndAddress, hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::SendEraseSector(uint32_t subSectorIndex)
    {
        infra::ByteRange instructionAndAddress = InstructionAndAddress(commandEraseSector, AddressOfSector(subSectorIndex));
        spi.SendData(instructionAndAddress, hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::SendEraseBulk()
    {
        spi.SendData(infra::MakeByteRange(commandEraseBulk), hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::HoldWhileWriteInProgress()
    {
        while ((ReadStatusRegister() & 1) == 1)
        {}
    }

    uint8_t SynchronousFlashSpi::ReadStatusRegister()
    {
        static const uint8_t instruction = commandReadStatusRegister;

        uint8_t statusRegister = 0;

        spi.SendData(infra::MakeByteRange(instruction), hal::SynchronousSpi::continueSession);
        spi.ReceiveData(infra::MakeByteRange(statusRegister), hal::SynchronousSpi::stop);

        return statusRegister;
    }
}
