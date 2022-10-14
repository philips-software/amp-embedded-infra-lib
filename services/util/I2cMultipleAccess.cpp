#include "services/util/I2cMultipleAccess.hpp"

namespace services
{
    I2cMultipleAccessMaster::I2cMultipleAccessMaster(hal::I2cMaster& master)
        : master(master)
    {}

    void I2cMultipleAccessMaster::SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent)
    {
        master.SendData(address, data, nextAction, onSent);
    }

    void I2cMultipleAccessMaster::ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result)> onReceived)
    {
        master.ReceiveData(address, data, nextAction, onReceived);
    }

    I2cMultipleAccess::I2cMultipleAccess(I2cMultipleAccessMaster& master)
        : master(master)
        , claimer(master)
    {}

    void I2cMultipleAccess::SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent)
    {
        this->onSent = onSent;
        if (!claimer.IsClaimed())
            claimer.Claim([this, address, data, nextAction, onSent]()
                { SendDataOnClaimed(address, data, nextAction); });
        else
            SendDataOnClaimed(address, data, nextAction);
    }

    void I2cMultipleAccess::SendDataOnClaimed(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction)
    {
        master.SendData(address, data, nextAction, [this, nextAction](hal::Result result, uint32_t numberOfBytesSent)
            {
            if (nextAction == hal::Action::stop)
                claimer.Release();
            this->onSent(result, numberOfBytesSent); });
    }

    void I2cMultipleAccess::ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction, infra::Function<void(hal::Result)> onReceived)
    {
        this->onReceived = onReceived;
        if (!claimer.IsClaimed())
            claimer.Claim([this, address, data, nextAction, onReceived]()
                { ReceiveDataOnClaimed(address, data, nextAction); });
        else
            ReceiveDataOnClaimed(address, data, nextAction);
    }

    void I2cMultipleAccess::ReceiveDataOnClaimed(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction)
    {
        master.ReceiveData(address, data, nextAction, [this, nextAction](hal::Result result)
            {
            if (nextAction == hal::Action::stop)
                claimer.Release();
            this->onReceived(result); });
    }
}
