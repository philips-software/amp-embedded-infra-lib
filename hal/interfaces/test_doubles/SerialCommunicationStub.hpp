#ifndef HAL_SERIAL_COMMUNICATION_STUB_HPP
#define HAL_SERIAL_COMMUNICATION_STUB_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <deque>

namespace hal
{
    class SerialCommunicationStub
        : public SerialCommunication
    {
    public:
        // SerialCommunication Interface
        virtual void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

        std::deque<uint8_t> dataReceivedByStub;

    private:
        infra::Function<void(infra::ConstByteRange data)> dataReceived;
        uint32_t bytesInRxBuffer;
        infra::ByteRange rxBufferMemoryRange;
    };
}

#endif
