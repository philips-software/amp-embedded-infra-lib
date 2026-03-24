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

        MOCK_METHOD(void, SendDataMock, (std::vector<uint8_t>));

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
        MOCK_METHOD(void, SendData, (infra::ConstByteRange data, infra::Function<void()> actionOnCompletion), (override));
        MOCK_METHOD(void, ReceiveData, (infra::Function<void(infra::ConstByteRange data)> dataReceived), (override));
    };

    class BufferedSerialCommunicationMock
        : public BufferedSerialCommunication
    {
    public:
        MOCK_METHOD(void, SendData, (infra::ConstByteRange data, infra::Function<void()> actionOnCompletion), (override));
        MOCK_METHOD(infra::StreamReaderWithRewinding&, Reader, (), (override));
        MOCK_METHOD(void, AckReceived, (), (override));
    };

    class BufferedSerialCommunicationObserverMock
        : public hal::BufferedSerialCommunicationObserver
    {
    public:
        using hal::BufferedSerialCommunicationObserver::BufferedSerialCommunicationObserver;

        MOCK_METHOD(void, DataReceived, (), (override));
    };
}

#endif
