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
        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone) override;
        virtual void SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator) override;
        virtual void SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator) override;
        virtual void ResetCommunicationConfigurator() override;

        virtual void StartSession() override;
        virtual void EndSession() override;

    private:
        hal::SpiMaster& spi;
        hal::OutputPin chipSelect;
        hal::ChipSelectConfigurator* chipSelectConfigurator = nullptr;
    };
}

#endif
