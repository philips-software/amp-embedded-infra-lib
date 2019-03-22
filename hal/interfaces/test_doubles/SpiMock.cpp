#include "infra/event/EventDispatcher.hpp"
#include "hal/interfaces/test_doubles/SpiMock.hpp"

namespace hal
{
    void SpiMock::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        if (!sendData.empty())
            SendDataMock(std::vector<uint8_t>(sendData.begin(), sendData.end()), nextAction);
        if (!receiveData.empty())
        {
            std::vector<uint8_t> dataToBeReceived = ReceiveDataMock(nextAction);
            EXPECT_EQ(dataToBeReceived.size(), receiveData.size());                                                     //TICS !CFL#001
            std::copy(dataToBeReceived.begin(), dataToBeReceived.end(), receiveData.begin());
        }

        infra::EventDispatcher::Instance().Schedule(onDone);
    }

    void SpiAsynchronousMock::SetChipSelectConfigurator(ChipSelectConfigurator& configurator)
    {
        chipSelectConfigurator = &configurator;
        SetChipSelectConfiguratorMock(configurator);
    }

    void SpiAsynchronousMock::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        this->onDone = onDone;
        SendAndReceiveMock(std::vector<uint8_t>(sendData.begin(), sendData.end()), receiveData, nextAction, onDone);
    }

    void SpiSlaveMock::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, const infra::Function<void()>& onDone)
    {
        SendAndReceiveMock(std::vector<uint8_t>(sendData.begin(), sendData.end()), receiveData, onDone);
    }
}
