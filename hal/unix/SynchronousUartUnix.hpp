#ifndef HAL_SYNCHRONOUS_UART_UNIX_HPP
#define HAL_SYNCHRONOUS_UART_UNIX_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
#include <string>

namespace hal
{
    namespace detail
    {
        struct SynchronousUartUnixConfig
        {
            uint32_t baudrate{ 115200 };
            bool flowControl{ false };
        };
    }

    class SynchronousUartUnix
        : public hal::SynchronousSerialCommunication
    {
    public:
        using Config = detail::SynchronousUartUnixConfig;

        explicit SynchronousUartUnix(const std::string& portName, const Config& config = {});
        virtual ~SynchronousUartUnix();

        // Implementation of hal::SynchronousSerialCommunication
        void SendData(infra::ConstByteRange data) override;
        bool ReceiveData(infra::ByteRange data) override;

    private:
        void Open(const std::string& portName, uint32_t baudrate, bool flowControl);

    private:
        int fileDescriptor{ -1 };
    };
}

#endif
