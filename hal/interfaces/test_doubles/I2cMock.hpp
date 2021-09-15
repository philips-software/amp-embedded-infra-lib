#ifndef HAL_I2C_MOCK_HPP
#define HAL_I2C_MOCK_HPP

#include "hal/interfaces/I2c.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class I2cMasterMock
        : public hal::I2cMaster
    {
    public:
        virtual void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        virtual void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;

        MOCK_METHOD3(SendDataMock, void(hal::I2cAddress address, hal::Action nextAction, std::vector<uint8_t> data));
        MOCK_METHOD2(ReceiveDataMock, std::vector<uint8_t>(hal::I2cAddress address, hal::Action nextAction));
    };

    class I2cMasterMockWithoutAutomaticDone
        : public hal::I2cMaster
    {
    public:
        virtual void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        virtual void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;

        MOCK_METHOD3(SendDataMock, void(hal::I2cAddress address, hal::Action nextAction, std::vector<uint8_t> data));
        MOCK_METHOD2(ReceiveDataMock, std::vector<uint8_t>(hal::I2cAddress address, hal::Action nextAction));

        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent;
        infra::Function<void(hal::Result)> onReceived;
    };

    class I2cSlaveMock
        : public hal::I2cSlave
    {
    public:
        MOCK_METHOD2(AddSlave, void(uint8_t address, infra::Function<void(DataDirection)> onAddressed));
        MOCK_METHOD2(SendData, void(infra::ConstByteRange data, infra::Function<void(Result, uint32_t numberOfBytesSent)> onSent));
        MOCK_METHOD3(ReceiveData, void(infra::ByteRange data, bool lastOfSession, infra::Function<void(Result, uint32_t numberOfBytesReceived)> onReceived));

        MOCK_METHOD1(RemoveSlave, void(uint8_t address));
        MOCK_METHOD0(StopTransceiving, void());
    };
}

#endif
