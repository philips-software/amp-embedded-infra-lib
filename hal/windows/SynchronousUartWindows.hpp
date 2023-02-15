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
    struct SynchronousUartWindowsConfig
    {
        enum class RtsFlowControl
        {
            RtsControlDisable = RTS_CONTROL_DISABLE,
            RtsControlEnable = RTS_CONTROL_ENABLE,
            RtsHandshake = RTS_CONTROL_HANDSHAKE,
            RtsControlToggle = RTS_CONTROL_TOGGLE
        };

        SynchronousUartWindowsConfig()
            : baudRate(CBR_115200)
            , flowControlRts(RtsFlowControl::RtsControlDisable)
        {}

        SynchronousUartWindowsConfig(uint32_t newbaudRate, RtsFlowControl newFlowControlRts)
            : baudRate(newbaudRate)
            , flowControlRts(newFlowControlRts)
        {}

        uint32_t baudRate;
        RtsFlowControl flowControlRts;
    };

    class SynchronousUartWindows
        : public SynchronousSerialCommunication
    {
    public:
        SynchronousUartWindows(const std::string& name, SynchronousUartWindowsConfig config = SynchronousUartWindowsConfig());
        ~SynchronousUartWindows();

        virtual void SendData(infra::ConstByteRange data) override;
        virtual bool ReceiveData(infra::ByteRange data) override;

    private:
        void Open(const std::string& name, SynchronousUartWindowsConfig config);

    private:
        void* handle = nullptr;
    };
}

#endif
