#ifndef HAL_I2C_REGISTER_ACCESS_MOCK_HPP
#define HAL_I2C_REGISTER_ACCESS_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/I2c.hpp"

namespace hal
{
    class I2cMasterRegisterAccessMock
        : public hal::I2cMaster
    {
    public:
        virtual void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        virtual void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;

        MOCK_METHOD1(ReadRegisterMock, std::vector<uint8_t>(uint8_t dataRegister));
        MOCK_METHOD2(WriteRegisterMock, void(uint8_t dataRegister, const std::vector<uint8_t>& data));

    private:
        bool sending = true;
        bool atStart = true;
        uint8_t dataRegister = 0;
        std::vector<uint8_t> currentSendData;
        std::vector<uint8_t> currentReceiveData;
    };
}

#endif
