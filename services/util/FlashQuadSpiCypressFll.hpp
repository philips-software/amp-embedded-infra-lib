#ifndef SERVICES_FLASH_QUAD_SPI_CYPRESS_FLL_HPP
#define SERVICES_FLASH_QUAD_SPI_CYPRESS_FLL_HPP

#include "services/util/FlashQuadSpi.hpp"

namespace services
{
    class FlashQuadSpiCypressFll
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
        static const uint8_t commandEnterQpi;
        static const uint8_t commandExitQpi;

        static const uint32_t sizeHalfBlock = 32768;

        static const uint8_t statusFlagWriteInProgress = 1;

        FlashQuadSpiCypressFll(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors = 4096);

    public:
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;

        void SwitchToSingleSpeed(infra::Function<void()> onDone);

    private:
        void SwitchToQuadSpeed();
        virtual void WriteEnable() override;
        virtual void EraseSomeSectors(uint32_t endIndex) override;
        void SendEraseSector(uint32_t sectorIndex);
        void SendEraseHalfBlock(uint32_t sectorIndex);
        void SendEraseBlock(uint32_t sectorIndex);
        void SendEraseChip();
        virtual void HoldWhileWriteInProgress() override;

    private:
        infra::TimerSingleShot initDelayTimer;
        infra::Function<void()> onInitialized;
    };
}

#endif
