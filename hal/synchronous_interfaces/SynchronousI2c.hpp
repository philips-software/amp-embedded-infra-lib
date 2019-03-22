#ifndef HAL_SYNCHRONOUS_I2C_HPP
#define HAL_SYNCHRONOUS_I2C_HPP

#include "hal/interfaces/I2c.hpp"
#include <cstdint>

namespace hal
{
    class SynchronousI2cMaster
    {
    public:
        struct SendResult
        {
            Result result;
            uint32_t numberOfBytesSent;
        };

        virtual SendResult SendData(I2cAddress address, infra::ConstByteRange data, Action nextAction) = 0;
        virtual Result ReceiveData(I2cAddress address, infra::ByteRange data, Action nextAction) = 0;
    };
}

#endif
