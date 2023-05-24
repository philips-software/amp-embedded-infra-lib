#ifndef HAL_LOW_POWER_SPI_MASTER
#define HAL_LOW_POWER_SPI_MASTER

#include "hal/interfaces/Spi.hpp"
#include "infra/event/LowPowerEventDispatcher.hpp"
#include "infra/util/AutoResetFunction.hpp"

namespace hal
{
    class LowPowerSpiMaster
        : public SpiMaster
    {
    public:
        LowPowerSpiMaster() = delete;
        LowPowerSpiMaster(hal::SpiMaster& master, infra::MainClockReference& mainClockReference);
        LowPowerSpiMaster(const LowPowerSpiMaster& other) = delete;
        LowPowerSpiMaster& operator=(const LowPowerSpiMaster& other) = delete;

        ~LowPowerSpiMaster() = default;

    public:
        void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, SpiAction nextAction, const infra::Function<void()>& onDone) override;
        void SetChipSelectConfigurator(ChipSelectConfigurator& configurator) override;
        void SetCommunicationConfigurator(CommunicationConfigurator& configurator) override;
        void ResetCommunicationConfigurator() override;

    private:
        infra::MainClockReference& mainClock;
        hal::SpiMaster& spiMaster;
        infra::AutoResetFunction<void()> onDone;
    };
}

#endif
