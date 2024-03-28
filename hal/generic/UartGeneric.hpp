#ifndef HAL_UART_HOST_HPP
#define HAL_UART_HOST_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <cstdint>
#include <string>

#if defined(EMIL_HAL_WINDOWS)
#include "hal/windows/UartWindows.hpp"
#elif defined(EMIL_HAL_UNIX)
#include "hal/unix/UartUnix.hpp"
#else
#error no host defined
#endif

namespace hal
{
    namespace detail
    {
        struct UartGenericConfig
        {
            enum class RtsFlowControl
            {
                none,
                enabled
            };

            uint32_t baudrate = 115200;
            RtsFlowControl rtsFlowControl = RtsFlowControl::none;
        };
    }

    class UartGeneric
        : public hal::SerialCommunication
    {
    public:
        using Config = detail::UartGenericConfig;

        UartGeneric(const std::string& portname, const Config& config = {});
        virtual ~UartGeneric() = default;

        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

    private:
#if defined(EMIL_HAL_WINDOWS)
        UartWindows::Config serialConfig;
        UartWindows serialImpl;
#elif defined(EMIL_HAL_UNIX)
        UartUnix::Config serialConfig;
        UartUnix serialImpl;
#endif
    };
}

#endif
