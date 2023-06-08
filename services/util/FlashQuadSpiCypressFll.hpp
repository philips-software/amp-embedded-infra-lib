#ifndef SERVICES_FLASH_QUAD_SPI_CYPRESS_FLL_HPP
#define SERVICES_FLASH_QUAD_SPI_CYPRESS_FLL_HPP

#include "hal/interfaces/FlashId.hpp"
#include "services/util/FlashQuadSpi.hpp"

namespace services
{
    class FlashQuadSpiCypressFll
        : public FlashQuadSpi
        , public hal::FlashId
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
        static const uint8_t commandReadUniqueId;

        static const uint32_t sizeHalfBlock = 32768;

        static const uint8_t statusFlagWriteInProgress = 1;

        FlashQuadSpiCypressFll(hal::QuadSpi& spi, infra::Function<void()> onInitialized, uint32_t numberOfSectors = 4096);

    public:
        void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;

        void SwitchToSingleSpeed(infra::Function<void()> onDone);

        // implement FlashId
        void ReadFlashId(infra::ByteRange buffer, infra::Function<void()> onDone) override;

    private:
        void SwitchToQuadSpeed();
        void WriteEnable() override;
        void EraseSomeSectors(uint32_t endIndex) override;
        void SendEraseSector(uint32_t sectorIndex);
        void SendEraseHalfBlock(uint32_t sectorIndex);
        void SendEraseBlock(uint32_t sectorIndex);
        void SendEraseChip();
        void HoldWhileWriteInProgress() override;

    private:
        infra::TimerSingleShot initDelayTimer;
        infra::Function<void()> onInitialized;
    };
}

#endif
