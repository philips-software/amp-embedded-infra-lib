#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"

namespace hal
{
    void SerialCommunicationMock::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        this->actionOnCompletion = actionOnCompletion;

        SendDataMock(std::vector<uint8_t>(data.begin(), data.end()));
    }

    void SerialCommunicationMock::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        this->dataReceived = dataReceived;
    }

    void SerialCommunicationMockWithAssert::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        assert(!this->actionOnCompletion);
        this->actionOnCompletion = actionOnCompletion;

        SendDataMock(std::vector<uint8_t>(data.begin(), data.end()));
    }
}
