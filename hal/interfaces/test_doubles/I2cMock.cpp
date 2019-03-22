#include "hal/interfaces/test_doubles/I2cMock.hpp"

namespace hal
{
    void I2cMasterMock::SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent)
    {
        SendDataMock(address, nextAction, std::vector<uint8_t>(data.begin(), data.end()));
        onSent(hal::Result::complete, data.size());
    }

    void I2cMasterMock::ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result)> onReceived)
    {
        std::vector<uint8_t> result = ReceiveDataMock(address, nextAction);
        assert(result.size() == data.size());
        std::copy(result.begin(), result.end(), data.begin());

        onReceived(hal::Result::complete);
    }

    void I2cMasterMockWithoutAutomaticDone::SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent)
    {
        SendDataMock(address, nextAction, std::vector<uint8_t>(data.begin(), data.end()));
        this->onSent = onSent;
    }

    void I2cMasterMockWithoutAutomaticDone::ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result)> onReceived)
    {
        std::vector<uint8_t> result = ReceiveDataMock(address, nextAction);
        assert(result.size() == data.size());
        std::copy(result.begin(), result.end(), data.begin());

        this->onReceived = onReceived;
    }
}
