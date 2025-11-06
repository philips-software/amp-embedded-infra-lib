#ifndef SERVICES_FLASH_SPI_HPP
#define SERVICES_FLASH_SPI_HPP

#include "hal/interfaces/FlashHomogeneous.hpp"
#include "hal/interfaces/FlashId.hpp"
#include "hal/interfaces/Spi.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Sequencer.hpp"

namespace services
{
    namespace detail
    {
        struct FlashSpiConfig
        {
            uint32_t nrOfSubSectors{ 512 };
            uint32_t sizeSector{ 65536 };
            uint32_t sizeSubSector{ 4096 };
            uint32_t sizePage{ 256 };
            bool extendedAddressing = false;
        };
    }

    class FlashSpi
        : public hal::FlashHomogeneous
        , public hal::FlashId
    {
    public:
        using Config = detail::FlashSpiConfig;

        static const std::array<uint8_t, 2> commandPageProgram;
        static const std::array<uint8_t, 2> commandReadData;
        static const std::array<uint8_t, 2> commandEraseSubSector;
        static const std::array<uint8_t, 2> commandEraseSector;
        static const uint8_t commandReadStatusRegister;
        static const uint8_t commandWriteEnable;
        static const uint8_t commandEraseBulk;
        static const uint8_t commandReadId;

        static const uint8_t statusFlagWriteInProgress = 1;

        explicit FlashSpi(hal::SpiMaster& spi, const Config& config = Config(), uint32_t timerId = infra::systemTimerServiceId);

    public:
        // implement Flash
        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;

        // implement FlashId
        void ReadFlashId(infra::ByteRange buffer, infra::Function<void()> onDone) override;

    private:
        void WriteEnable();
        void PageProgram();
        void EraseSomeSectors(uint32_t endIndex);
        void SendEraseSubSector(uint32_t subSectorIndex);
        void SendEraseSector(uint32_t subSectorIndex);
        void SendEraseBulk();
        void HoldWhileWriteInProgress();
        void ReadStatusRegister();
        infra::ConstByteRange InstructionAndAddress(const std::array<uint8_t, 2>& instruction, uint32_t address);

    protected:
        infra::ClaimableResource& Resource();
        hal::SpiMaster& Spi();

    private:
        hal::SpiMaster& spi;
        infra::ClaimableResource resource;

        struct LargestLambdaCapture
        {
            void* thisPtr;
            infra::Function<void()> onDone;
            infra::ByteRange buffer;
        };

        infra::ClaimableResource::Claimer::WithSize<sizeof(LargestLambdaCapture)> flashOperationClaimer{ resource };
        infra::ClaimableResource::Claimer::WithSize<sizeof(LargestLambdaCapture)> flashIdClaimer{ resource };
        const Config config;
        infra::Sequencer sequencer;
        infra::TimerSingleShot delayTimer;
        infra::AutoResetFunction<void()> onDone;
        infra::ConstByteRange buffer;
        infra::ConstByteRange writeBuffer;
        infra::ByteRange readBuffer;
        uint32_t address = 0;
        uint32_t sectorIndex = 0;
        uint8_t statusRegister = 0;
        std::array<uint8_t, 5> instructionAndAddressBuffer;
    };
}

#endif
