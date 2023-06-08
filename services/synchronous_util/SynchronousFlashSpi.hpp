#ifndef SERVICES_SYNCHRONOUS_FLASH_SPI_HPP
#define SERVICES_SYNCHRONOUS_FLASH_SPI_HPP

#include "hal/synchronous_interfaces/SynchronousFlashHomogeneous.hpp"
#include "hal/synchronous_interfaces/SynchronousFlashId.hpp"
#include "hal/synchronous_interfaces/SynchronousSpi.hpp"

namespace services
{
    struct SynchronousFlashSpiConfig
    {
        uint32_t numberOfSubSectors = 512;
        bool extendedAddressing = false;
    };

    class SynchronousFlashSpi
        : public hal::SynchronousFlashHomogeneous
        , public hal::SynchronousFlashId
    {
    public:
        using Config = SynchronousFlashSpiConfig;

        static const uint8_t commandPageProgram[2];
        static const uint8_t commandReadData[2];
        static const uint8_t commandEraseSubSector[2];
        static const uint8_t commandEraseSector[2];
        static const uint8_t commandReadStatusRegister;
        static const uint8_t commandWriteEnable;
        static const uint8_t commandEraseBulk;
        static const uint8_t commandReadId;

        static const uint32_t sizeSector = 65536;
        static const uint32_t sizeSubSector = 4096;
        static const uint32_t sizePage = 256;

        static const uint8_t statusFlagWriteInProgress = 1;

        SynchronousFlashSpi(hal::SynchronousSpi& spi, const Config& config = Config());

    public:
        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex) override;

        void ReadFlashId(infra::ByteRange buffer) override;

    private:
        void WriteEnable();
        void PageProgram();
        void EraseSomeSectors(uint32_t endIndex);
        void SendEraseSubSector(uint32_t subSectorIndex);
        void SendEraseSector(uint32_t subSectorIndex);
        void SendEraseBulk();
        void HoldWhileWriteInProgress();
        uint8_t ReadStatusRegister();

        infra::ByteRange InstructionAndAddress(const uint8_t instruction[], uint32_t address);

    private:
        hal::SynchronousSpi& spi;
        Config config;
        infra::ConstByteRange buffer;
        uint32_t address = 0;
        uint32_t sectorIndex = 0;

        std::array<uint8_t, 5> instructionAndAddressBuffer;
    };
}

#endif
