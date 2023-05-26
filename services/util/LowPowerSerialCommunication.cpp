#include "services/util/LowPowerSerialCommunication.hpp"

namespace hal
{
    LowPowerSerialCommunication::LowPowerSerialCommunication(hal::SerialCommunication& serialCommunication, infra::MainClockReference& mainClockReference)
        : mainClock(mainClockReference)
        , serialCommunication(serialCommunication)
    {}

    void LowPowerSerialCommunication::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        mainClock.Refere();
        this->actionOnCompletion = actionOnCompletion;
        serialCommunication.SendData(data, [this]()
            {
            this->actionOnCompletion();
            this->mainClock.Release(); });
    }

    void LowPowerSerialCommunication::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        serialCommunication.ReceiveData(dataReceived);
    }
}
