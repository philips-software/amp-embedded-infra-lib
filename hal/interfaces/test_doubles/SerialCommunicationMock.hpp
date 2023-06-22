#ifndef HAL_SERIAL_COMMUNICATION_MOCK_HPP
#define HAL_SERIAL_COMMUNICATION_MOCK_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class SerialCommunicationMock
        : public SerialCommunication
    {
    public:
        // SerialCommunication Interface
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

        MOCK_METHOD1(SendDataMock, void(std::vector<uint8_t>));

        infra::AutoResetFunction<void()> actionOnCompletion;
        infra::Function<void(infra::ConstByteRange data)> dataReceived;
    };

    class SerialCommunicationMockWithAssert
        : public SerialCommunicationMock
    {
    public:
        // SerialCommunication Interface
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
    };

    class SerialCommunicationCleanMock
        : public SerialCommunication
    {
    public:
        // SerialCommunication Interface
        MOCK_METHOD2(SendData, void(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion));
        MOCK_METHOD1(ReceiveData, void(infra::Function<void(infra::ConstByteRange data)> dataReceived));
    };
}

#endif
