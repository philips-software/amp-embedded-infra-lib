#ifndef HAL_UART_UNIX_BASE_HPP
#define HAL_UART_UNIX_BASE_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
#include <string>

namespace hal
{
    namespace detail
    {
        struct UartUnixBaseConfig
        {
            uint32_t baudrate{ 115200 };
            bool flowControl{ false };
        };
    }

    class PortNotOpened
        : public std::runtime_error
    {
    public:
        PortNotOpened(const std::string& portName, const std::string& errstr);
    };

    class UartUnixBase
    {
    public:
        using Config = detail::UartUnixBaseConfig;

        explicit UartUnixBase(const std::string& portName, const Config& config = {});
        virtual ~UartUnixBase();

    private:
        void Open(const std::string& portName, uint32_t baudrate, bool flowControl);

    protected:
        [[nodiscard]] int FileDescriptor() const;

    private:
        int fileDescriptor{ -1 };
    };
}

#endif
