#ifndef HAL_SYNCHRONOUS_UART_WINDOWS_HPP
#define HAL_SYNCHRONOUS_UART_WINDOWS_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"

#include <winsock2.h>
#include <windows.h>
#include <setupapi.h>

namespace hal
{
	class SynchronousUartWindows : public SynchronousSerialCommunication
	{
	public:
        struct UartWindowsConfig
        {
            enum class RtsFlowControl
            {
                RtsControlDisable = RTS_CONTROL_DISABLE,
                RtsControlEnable = RTS_CONTROL_ENABLE,
                RtsHandshake = RTS_CONTROL_HANDSHAKE,
                RtsControlToggle = RTS_CONTROL_TOGGLE
            };

            UartWindowsConfig()
                : baudRate(CBR_115200)
                , flowControlRts(RtsFlowControl::RtsControlDisable) {}

            UartWindowsConfig(uint32_t newbaudRate, RtsFlowControl newFlowControlRts)
                : baudRate(newbaudRate)
                , flowControlRts(newFlowControlRts)
            {}

            uint32_t baudRate;
            RtsFlowControl flowControlRts;
        };

        SynchronousUartWindows(const std::string& name, UartWindowsConfig config = UartWindowsConfig());
        ~SynchronousUartWindows();

		virtual void SendData(infra::ConstByteRange data) override;
		virtual bool ReceiveData(infra::ByteRange data) override;

    private:
        void Open(const std::string& name, UartWindowsConfig config);

    private:

        void* handle = nullptr;
	};
}

#endif
