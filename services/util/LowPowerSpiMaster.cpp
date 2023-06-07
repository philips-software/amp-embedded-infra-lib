#include "services/util/LowPowerSpiMaster.hpp"

namespace hal
{
    LowPowerSpiMaster::LowPowerSpiMaster(hal::SpiMaster& master, infra::MainClockReference& mainClockReference)
        : mainClock(mainClockReference)
        , spiMaster(master)
    {}

    void LowPowerSpiMaster::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        mainClock.Refere();
        this->onDone = onDone;

        spiMaster.SendAndReceive(sendData, receiveData, nextAction, [this]()
            {
            this->onDone();
            this->mainClock.Release(); });
    }

    void LowPowerSpiMaster::SetChipSelectConfigurator(ChipSelectConfigurator& configurator)
    {
        spiMaster.SetChipSelectConfigurator(configurator);
    }

    void LowPowerSpiMaster::SetCommunicationConfigurator(CommunicationConfigurator& configurator)
    {
        spiMaster.SetCommunicationConfigurator(configurator);
    }

    void LowPowerSpiMaster::ResetCommunicationConfigurator()
    {
        spiMaster.ResetCommunicationConfigurator();
    }
}
