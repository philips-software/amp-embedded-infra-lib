#include "services/util/SpiMasterWithChipSelect.hpp"

namespace services
{
    SpiMasterWithChipSelect::SpiMasterWithChipSelect(hal::SpiMaster& spi, hal::GpioPin& chipSelect)
        : spi(spi)
        , chipSelect(chipSelect, true)
    {
        spi.SetChipSelectConfigurator(*this);
    }

    void SpiMasterWithChipSelect::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        spi.SendAndReceive(sendData, receiveData, nextAction, onDone);
    }

    void SpiMasterWithChipSelect::SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator)
    {
        chipSelectConfigurator = &configurator;
    }

    void SpiMasterWithChipSelect::SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator)
    {
        spi.SetCommunicationConfigurator(configurator);
    }

    void SpiMasterWithChipSelect::ResetCommunicationConfigurator()
    {
        spi.ResetCommunicationConfigurator();
    }

    void SpiMasterWithChipSelect::StartSession()
    {
        if (chipSelectConfigurator != nullptr)
            chipSelectConfigurator->StartSession();

        chipSelect.Set(false);
    }

    void SpiMasterWithChipSelect::EndSession()
    {
        chipSelect.Set(true);

        if (chipSelectConfigurator != nullptr)
            chipSelectConfigurator->EndSession();
    }
}
