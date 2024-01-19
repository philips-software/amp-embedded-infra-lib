#ifndef HAL_SYNCHRONOUS_UART_WINDOWS_HPP
#define HAL_SYNCHRONOUS_UART_WINDOWS_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <setupapi.h>

// clang-format on

namespace hal
{
    namespace details
    {
        struct SynchronousUartWindowsConfig
        {
            enum class RtsFlowControl
            {
                RtsControlDisable = RTS_CONTROL_DISABLE,
                RtsControlEnable = RTS_CONTROL_ENABLE,
                RtsHandshake = RTS_CONTROL_HANDSHAKE,
                RtsControlToggle = RTS_CONTROL_TOGGLE
            };

            uint32_t baudRate{ CBR_115200 };
            RtsFlowControl flowControlRts{ RtsFlowControl::RtsControlDisable };
        };
    }

    class SynchronousUartWindows
        : public SynchronousSerialCommunication
    {
    public:
        using Config = details::SynchronousUartWindowsConfig;

        SynchronousUartWindows(const std::string& name, Config config = {});
        ~SynchronousUartWindows();

        void SendData(infra::ConstByteRange data) override;
        bool ReceiveData(infra::ByteRange data) override;

    private:
        void Open(const std::string& name, Config config);

    private:
        void* handle = nullptr;
    };
}

#endif
