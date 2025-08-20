#ifndef SERVICES_SERIAL_COMMUNICATION_ON_SEGGER_RTT_HPP
#define SERVICES_SERIAL_COMMUNICATION_ON_SEGGER_RTT_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/timer/Timer.hpp"
#include <chrono>

namespace services
{
    class SerialCommunicationOnSeggerRtt
        : public hal::SerialCommunication
    {
    public:
        SerialCommunicationOnSeggerRtt(unsigned int bufferIndex = 0, infra::Duration readInterval = std::chrono::milliseconds(20));

        void SendData(infra::ConstByteRange range, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

    private:
        unsigned int bufferIndex;
        infra::Duration readInterval;
        static inline bool initialized{ false };

        infra::Function<void(infra::ConstByteRange data)> dataReceived;
        infra::TimerRepeating readTimer;
    };
}

#endif
