#ifndef SERVICES_FLASH_QUAD_SPI_HPP
#define SERVICES_FLASH_QUAD_SPI_HPP

#include "hal/interfaces/FlashHomogeneous.hpp"
#include "hal/interfaces/QuadSpi.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/Sequencer.hpp"

namespace services
{
    class FlashQuadSpi
        : public hal::FlashHomogeneous
    {
    public:
        static const uint32_t sizeBlock = 65536;
        static const uint32_t sizeSector = 4096;
        static const uint32_t sizePage = 256;

    public:
        FlashQuadSpi(hal::QuadSpi& spi, uint32_t numberOfSectors, uint8_t commandPageProgram);

        virtual void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;

    protected:
        void WriteBufferSequence();
        virtual void PageProgram();
        infra::BoundedVector<uint8_t>::WithMaxSize<4> ConvertAddress(uint32_t address) const;

        virtual void WriteEnable() = 0;
        virtual void EraseSomeSectors(uint32_t endIndex) = 0;
        virtual void HoldWhileWriteInProgress() = 0;

    protected:
        hal::QuadSpi& spi;
        infra::Sequencer sequencer;
        infra::AutoResetFunction<void()> onDone;
        infra::ConstByteRange buffer;
        uint32_t address = 0;
        uint32_t sectorIndex = 0;
        uint8_t commandPageProgram;
    };
}

#endif
