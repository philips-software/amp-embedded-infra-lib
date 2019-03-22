#ifndef HAL_LOW_POWER_SERIAL_COMMUNICATION_HPP
#define HAL_LOW_POWER_SERIAL_COMMUNICATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/LowPowerEventDispatcher.hpp"
#include "infra/util/AutoResetFunction.hpp"

namespace hal
{
    class LowPowerSerialCommunication
        : public SerialCommunication
    {
    public:
        LowPowerSerialCommunication(hal::SerialCommunication& serialCommunication, infra::MainClockReference& mainClockReference);

        virtual void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;
    
    private:
        infra::MainClockReference& mainClock;
        hal::SerialCommunication& serialCommunication;
        infra::AutoResetFunction<void()> actionOnCompletion;
    };
}

#endif 
