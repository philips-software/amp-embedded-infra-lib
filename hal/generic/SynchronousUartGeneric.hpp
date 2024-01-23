#ifndef HAL_UART_HOST_HPP
#define HAL_UART_HOST_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
#include <cstdint>
#include <string>

#if defined(EMIL_HAL_WINDOWS)
#include "hal/windows/SynchronousUartWindows.hpp"
#elif defined(EMIL_HAL_UNIX)
#include "hal/unix/SynchronousUartUnix.hpp"
#else
#error no host defined
#endif

namespace hal
{
    namespace detail
    {
        struct SynchronousUartGenericConfig
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

    class SynchronousUartGeneric
        : public hal::SynchronousSerialCommunication
    {
    public:
        using Config = detail::SynchronousUartGenericConfig;

        SynchronousUartGeneric(const std::string& portname, const Config& config = {});
        virtual ~SynchronousUartGeneric() = default;

        void SendData(infra::ConstByteRange data) override;
        bool ReceiveData(infra::ByteRange data) override;

    private:
#if defined(EMIL_HAL_WINDOWS)
        SynchronousUartWindows::Config serialConfig;
        SynchronousUartWindows serialImpl;
#elif defined(EMIL_HAL_UNIX)
        SynchronousUartUnix::Config serialConfig;
        SynchronousUartUnix serialImpl;
#endif
    };
}

#endif
