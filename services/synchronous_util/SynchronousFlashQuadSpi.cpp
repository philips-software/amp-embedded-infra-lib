#include "services/synchronous_util/SynchronousFlashQuadSpi.hpp"

namespace services
{
    SynchronousFlashQuadSpi::SynchronousFlashQuadSpi(hal::SynchronousQuadSpi& quadSpi)
        : hal::SynchronousFlashHomogeneous(4096, 4096)
        , quadSpi(quadSpi)
    {
        // Switch to single line speed
        const uint8_t commandWriteEnable = 0x06;
        static const hal::SynchronousQuadSpi::Header writeEnableHeader{ commandWriteEnable, std::numeric_limits<uint32_t>::max(), 0 };
        quadSpi.SendData(writeEnableHeader, infra::ByteRange());

        const uint8_t commandWriteEnhancedVolatileRegister = 0x61;
        static const hal::SynchronousQuadSpi::Header writeVolatileRegisterHeader{ commandWriteEnhancedVolatileRegister, std::numeric_limits<uint32_t>::max(), 0 };
        const uint8_t volatileRegisterForQuadSpeed = 0x5f;
        quadSpi.SendData(writeVolatileRegisterHeader, infra::MakeByteRange(volatileRegisterForQuadSpeed));
    }

    void SynchronousFlashQuadSpi::WriteBuffer(infra::ConstByteRange buffer_, uint32_t address_)
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

    void SynchronousFlashQuadSpi::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        const uint8_t commandReadData = 0x0B;
        const hal::SynchronousQuadSpi::Header header{ commandReadData, address, 10 };

        quadSpi.ReceiveDataQuad(header, buffer);
    }

    void SynchronousFlashQuadSpi::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {}

    void SynchronousFlashQuadSpi::WriteEnable()
    {
        const uint8_t commandWriteEnable = 0x06;
        static const hal::SynchronousQuadSpi::Header writeEnableHeader{ commandWriteEnable, std::numeric_limits<uint32_t>::max(), 0 };
        quadSpi.SendDataQuad(writeEnableHeader, infra::ByteRange());
    }

    void SynchronousFlashQuadSpi::PageProgram()
    {
        infra::ConstByteRange currentBuffer = infra::Head(buffer, sizePage - AddressOffsetInSector(address) % sizePage);
        buffer.pop_front(currentBuffer.size());

        const uint8_t commandPageProgram = 0x32;
        hal::SynchronousQuadSpi::Header pageProgramHeader{ commandPageProgram, address, 0 };
        quadSpi.SendDataQuad(pageProgramHeader, currentBuffer);
    }

    void SynchronousFlashQuadSpi::HoldWhileWriteInProgress()
    {
        while ((ReadStatusRegister() & 1) == 1)
        {}
    }

    uint8_t SynchronousFlashQuadSpi::ReadStatusRegister()
    {
        uint8_t result;
        const uint8_t commandReadStatusRegister = 0x05;
        static const hal::SynchronousQuadSpi::Header readStatusHeader{ commandReadStatusRegister, std::numeric_limits<uint32_t>::max(), 0 };
        quadSpi.ReceiveDataQuad(readStatusHeader, infra::MakeByteRange(result));
        return result;
    }
}
