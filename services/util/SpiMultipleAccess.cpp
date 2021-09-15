#include "services/util/SpiMultipleAccess.hpp"

namespace services
{
    SpiMultipleAccessMaster::SpiMultipleAccessMaster(hal::SpiMaster& master)
        : master(master)
    {}

    void SpiMultipleAccessMaster::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        master.SendAndReceive(sendData, receiveData, nextAction, onDone);
    }

    void SpiMultipleAccessMaster::SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator)
    {
        master.SetChipSelectConfigurator(configurator);
    }

    void SpiMultipleAccessMaster::SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator)
    {
        if (communicationConfigurator != &configurator)
        {
            master.SetCommunicationConfigurator(configurator);
            communicationConfigurator = &configurator;
        }
    }

    void SpiMultipleAccessMaster::ResetCommunicationConfigurator()
    {
        if (communicationConfigurator != nullptr)
        {
            master.ResetCommunicationConfigurator();
            communicationConfigurator = nullptr;
        }
    }

    SpiMultipleAccess::SpiMultipleAccess(SpiMultipleAccessMaster& master)
        : master(master)
        , claimer(master)
    {}

    void SpiMultipleAccess::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        if (!claimer.IsClaimed())
            claimer.Claim([this, sendData, receiveData, nextAction, onDone]() {
                SendAndReceiveOnClaimed(sendData, receiveData, nextAction, onDone);
            });
        else
            SendAndReceiveOnClaimed(sendData, receiveData, nextAction, onDone);
    }

    void SpiMultipleAccess::SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator)
    {
        chipSelectConfigurator = &configurator;
    }

    void SpiMultipleAccess::SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator)
    {
        communicationConfigurator = &configurator;
    }

    void SpiMultipleAccess::ResetCommunicationConfigurator()
    {
        communicationConfigurator = nullptr;
    }

    void SpiMultipleAccess::StartSession()
    {
        if (chipSelectConfigurator != nullptr)
            chipSelectConfigurator->StartSession();
    }

    void SpiMultipleAccess::EndSession()
    {
        if (chipSelectConfigurator != nullptr)
            chipSelectConfigurator->EndSession();

        claimer.Release();
    }

    void SpiMultipleAccess::SendAndReceiveOnClaimed(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        if (communicationConfigurator != nullptr)
            master.SetCommunicationConfigurator(*communicationConfigurator);
        else
            master.ResetCommunicationConfigurator();

        master.SetChipSelectConfigurator(*this);
        master.SendAndReceive(sendData, receiveData, nextAction, onDone);
    }
}
