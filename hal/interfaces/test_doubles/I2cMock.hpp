#ifndef HAL_I2C_MOCK_HPP
#define HAL_I2C_MOCK_HPP

#include "hal/interfaces/I2c.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class I2cErrorPolicyMock
        : public hal::I2cErrorPolicy
    {
    public:
        MOCK_METHOD(void, DeviceNotFound, (), (override));
        MOCK_METHOD(void, BusError, (), (override));
        MOCK_METHOD(void, ArbitrationLost, (), (override));
    };

    class I2cMasterMock
        : public hal::I2cMaster
    {
    public:
        void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;
        MOCK_METHOD(void, SetErrorPolicy, (hal::I2cErrorPolicy & policy), (override));
        MOCK_METHOD(void, ResetErrorPolicy, (), (override));

        MOCK_METHOD(void, SendDataMock, (hal::I2cAddress address, hal::Action nextAction, std::vector<uint8_t> data));
        MOCK_METHOD(std::vector<uint8_t>, ReceiveDataMock, (hal::I2cAddress address, hal::Action nextAction));
    };

    class I2cMasterMockWithoutAutomaticDone
        : public hal::I2cMaster
    {
    public:
        void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;
        MOCK_METHOD(void, SetErrorPolicy, (hal::I2cErrorPolicy & policy), (override));
        MOCK_METHOD(void, ResetErrorPolicy, (), (override));

        MOCK_METHOD(void, SendDataMock, (hal::I2cAddress address, hal::Action nextAction, std::vector<uint8_t> data));
        MOCK_METHOD(std::vector<uint8_t>, ReceiveDataMock, (hal::I2cAddress address, hal::Action nextAction));

        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent;
        infra::Function<void(hal::Result)> onReceived;
    };

    class I2cSlaveMock
        : public hal::I2cSlave
    {
    public:
        MOCK_METHOD(void, AddSlave, (uint8_t address, infra::Function<void(DataDirection)> onAddressed));
        MOCK_METHOD(void, SendData, (infra::ConstByteRange data, infra::Function<void(Result, uint32_t numberOfBytesSent)> onSent));
        MOCK_METHOD(void, ReceiveData, (infra::ByteRange data, bool lastOfSession, infra::Function<void(Result, uint32_t numberOfBytesReceived)> onReceived));

        MOCK_METHOD(void, RemoveSlave, (uint8_t address));
        MOCK_METHOD(void, StopTransceiving, ());
    };
}

#endif
