#ifndef SERVICES_FLASH_QUAD_SPI_MICRON_N25Q_HPP
#define SERVICES_FLASH_QUAD_SPI_MICRON_N25Q_HPP

#include "services/util/FlashQuadSpi.hpp"

namespace services
{
    class FlashQuadSpiMicronN25q
        : public FlashQuadSpi
    {
    public:
        static const uint8_t commandPageProgram;
        static const uint8_t commandReadData;
        static const uint8_t commandReadStatusRegister;
        static const uint8_t commandReadFlagStatusRegister;
        static const uint8_t commandWriteEnable;
        static const uint8_t commandEraseSubSector;
        static const uint8_t commandEraseSector;
        static const uint8_t commandEraseBulk;
        static const uint8_t commandWriteEnhancedVolatileRegister;
        static const uint8_t commandReadEnhancedVolatileRegister;

        static const uint8_t statusFlagWriteInProgress = 1;
        static const uint8_t volatileRegisterForQuadSpeed;

        FlashQuadSpiMicronN25q(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors = 4096);

    public:
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;

    private:
        void WriteEnableSingleSpeed();
        void SwitchToQuadSpeed();
        virtual void WriteEnable() override;
        virtual void EraseSomeSectors(uint32_t endIndex) override;
        void SendEraseSubSector(uint32_t subSectorIndex);
        void SendEraseSector(uint32_t subSectorIndex);
        void SendEraseBulk();
        virtual void HoldWhileWriteInProgress() override;

    private:
        infra::TimerSingleShot initDelayTimer;
        infra::Function<void()> onInitialized;
    };
}

#endif
