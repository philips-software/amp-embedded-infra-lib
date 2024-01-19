
#include "hal/generic/SynchronousUartGeneric.hpp"
#include <cstdint>
#include <string>

#if defined(EMIL_HAL_WINDOWS)
#include "hal/windows/UartWindows.hpp"
#endif

#if defined(EMIL_HAL_UNIX)
#include "hal/unix/UartUnix.hpp"
#endif

namespace hal
{
    SynchronousUartGeneric::SynchronousUartGeneric(const std::string& portname, const Config& config)
#if defined(EMIL_HAL_WINDOWS)
        : serialConfig{ config.baudrate, config.rtsFlowControl == Config::RtsFlowControl::none ? SynchronousUartWindows::Config::RtsFlowControl::RtsControlDisable : SynchronousUartWindows::Config::RtsFlowControl::RtsHandshake }
        , serialImpl{ portname, serialConfig }
#endif
#if defined(EMIL_HAL_UNIX)
        : serialConfig{ config.baudrate, config.rtsFlowControl != Config::RtsFlowControl::none }
        , serialImpl{ portname, serialConfig }
#endif
    {}

    void SynchronousUartGeneric::SendData(infra::ConstByteRange data)
    {
        serialImpl.SendData(data);
    }

    bool SynchronousUartGeneric::ReceiveData(infra::ByteRange data)
    {
        return serialImpl.ReceiveData(data);
    }
}
