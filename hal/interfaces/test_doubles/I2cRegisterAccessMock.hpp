#ifndef HAL_I2C_REGISTER_ACCESS_MOCK_HPP
#define HAL_I2C_REGISTER_ACCESS_MOCK_HPP

#include "hal/interfaces/I2c.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class I2cMasterRegisterAccessMock
        : public hal::I2cMaster
    {
    public:
        void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;

        MOCK_METHOD(std::vector<uint8_t>, ReadRegisterMock, (uint8_t dataRegister));
        MOCK_METHOD(void, WriteRegisterMock, (uint8_t dataRegister, const std::vector<uint8_t>& data));
        MOCK_METHOD(void, SetErrorPolicy, (hal::I2cErrorPolicy & policy), (override));
        MOCK_METHOD(void, ResetErrorPolicy, (), (override));

    private:
        bool sending = true;
        bool atStart = true;
        uint8_t dataRegister = 0;
        std::vector<uint8_t> currentSendData;
        std::vector<uint8_t> currentReceiveData;
    };
}

#endif
