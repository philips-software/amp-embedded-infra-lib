#ifndef SERVICES_FLASH_QUAD_SPI_SINGLE_SPEED_HPP
#define SERVICES_FLASH_QUAD_SPI_SINGLE_SPEED_HPP

#include "services/util/FlashQuadSpi.hpp"

namespace services
{
    class FlashQuadSpiSingleSpeed
        : public FlashQuadSpi
    {
    public:
        static const uint8_t commandPageProgram;
        static const uint8_t commandReadData;
        static const uint8_t commandReadStatusRegister;
        static const uint8_t commandWriteEnable;
        static const uint8_t commandEraseSector;
        static const uint8_t commandEraseHalfBlock;
        static const uint8_t commandEraseBlock;
        static const uint8_t commandEraseChip;

        static const uint8_t statusFlagWriteInProgress = 1;

        FlashQuadSpiSingleSpeed(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors = 4096);

    public:
        void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;

    protected:
        void PageProgram() override;

    private:
        void WriteEnable() override;
        void EraseSomeSectors(uint32_t endIndex) override;
        void SendEraseSector(uint32_t sectorIndex);
        void SendEraseBlock(uint32_t sectorIndex);
        void SendEraseChip();
        void HoldWhileWriteInProgress() override;

    private:
        infra::Function<void()> onInitialized;
        infra::TimerSingleShot initDelayTimer;
    };
}

#endif
