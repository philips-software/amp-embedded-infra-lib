#ifndef SERVICES_SPI_MASTER_WITH_CHIP_SELECT_HPP
#define SERVICES_SPI_MASTER_WITH_CHIP_SELECT_HPP

#include "hal/interfaces/Gpio.hpp"
#include "hal/interfaces/Spi.hpp"

namespace services
{
    class SpiMasterWithChipSelect
        : public hal::SpiMaster
        , public hal::ChipSelectConfigurator
    {
    public:
        SpiMasterWithChipSelect(hal::SpiMaster& aSpi, hal::GpioPin& aChipSelect);

    public:
        void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone) override;
        void SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator) override;
        void SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator) override;
        void ResetCommunicationConfigurator() override;

        void StartSession() override;
        void EndSession() override;

    private:
        hal::SpiMaster& spi;
        hal::OutputPin chipSelect;
        hal::ChipSelectConfigurator* chipSelectConfigurator = nullptr;
    };
}

#endif
