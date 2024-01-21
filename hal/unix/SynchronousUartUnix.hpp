#ifndef HAL_SYNCHRONOUS_UART_UNIX_HPP
#define HAL_SYNCHRONOUS_UART_UNIX_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
#include "hal/unix/UartUnixBase.hpp"
#include <string>

namespace hal
{

    class SynchronousUartUnix
        : public UartUnixBase
        , public hal::SynchronousSerialCommunication
    {
    public:
        using Config = UartUnixBase::Config;

        using UartUnixBase::UartUnixBase;

        // Implementation of hal::SynchronousSerialCommunication
        void SendData(infra::ConstByteRange data) override;
        bool ReceiveData(infra::ByteRange data) override;
    };
}

#endif
