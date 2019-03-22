#include "services/synchronous_util/SynchronousFlashSpi.hpp"

namespace services
{
    const uint8_t SynchronousFlashSpi::commandPageProgram = 0x02;
    const uint8_t SynchronousFlashSpi::commandReadData = 0x03;
    const uint8_t SynchronousFlashSpi::commandReadStatusRegister = 0x05;
    const uint8_t SynchronousFlashSpi::commandWriteEnable = 0x06;
    const uint8_t SynchronousFlashSpi::commandEraseSubSector = 0x20;
    const uint8_t SynchronousFlashSpi::commandEraseSector = 0xd8;
    const uint8_t SynchronousFlashSpi::commandEraseBulk = 0xc7;

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
        instructionAndAddress.instruction = commandReadData;
        instructionAndAddress.address = ConvertAddress(address);
        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SynchronousSpi::continueSession);
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

    std::array<uint8_t, 3> SynchronousFlashSpi::ConvertAddress(uint32_t address) const
    {
        uint32_t linearAddress = address;
        std::array<uint8_t, 3> result;
        result[0] = static_cast<uint8_t>(linearAddress >> 16);
        result[1] = static_cast<uint8_t>(linearAddress >> 8);
        result[2] = static_cast<uint8_t>(linearAddress);
        return result;
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

        instructionAndAddress.instruction = commandPageProgram;
        instructionAndAddress.address = ConvertAddress(address);
        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SynchronousSpi::continueSession);
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
        instructionAndAddress.instruction = commandEraseSubSector;
        instructionAndAddress.address = ConvertAddress(AddressOfSector(subSectorIndex));

        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::SendEraseSector(uint32_t subSectorIndex)
    {
        instructionAndAddress.instruction = commandEraseSector;
        instructionAndAddress.address = ConvertAddress(AddressOfSector(subSectorIndex));

        spi.SendData(infra::MakeByteRange(instructionAndAddress), hal::SynchronousSpi::stop);
    }

    void SynchronousFlashSpi::SendEraseBulk()
    {
        instructionAndAddress.instruction = commandEraseBulk;

        spi.SendData(infra::MakeByteRange(instructionAndAddress.instruction), hal::SynchronousSpi::stop);
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
