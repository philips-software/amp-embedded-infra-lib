
#include "hal/generic/UartGeneric.hpp"
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
    UartGeneric::UartGeneric(const std::string& portname, const Config& config)
#if defined(EMIL_HAL_WINDOWS)
        : serialConfig{ config.baudrate, config.rtsFlowControl == Config::RtsFlowControl::none ? UartWindows::Config::RtsFlowControl::RtsControlDisable : UartWindows::Config::RtsFlowControl::RtsHandshake }
        , serialImpl{ portname, serialConfig }
#endif
#if defined(EMIL_HAL_UNIX)
        : serialConfig{ config.baudrate, config.rtsFlowControl != Config::RtsFlowControl::none }
        , serialImpl{ portname, serialConfig }
#endif
    {}

    void UartGeneric::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        serialImpl.SendData(data, actionOnCompletion);
    }

    void UartGeneric::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        serialImpl.ReceiveData(dataReceived);
    }
}
