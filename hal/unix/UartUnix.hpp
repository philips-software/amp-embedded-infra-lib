#ifndef HAL_UART_UNIX_HPP
#define HAL_UART_UNIX_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "hal/unix/UartUnixBase.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace hal
{
    class UartUnix
        : public UartUnixBase
        , public hal::SerialCommunication
    {
    public:
        using Config = UartUnixBase::Config;

        explicit UartUnix(const std::string& portName, const Config& config = {});
        ~UartUnix() override;

        // Implementation of hal::SerialCommunication
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

    private:
        void Read();

    private:
        uint8_t buffer;
        std::atomic<bool> running{ false };
        std::thread readThread;
        std::mutex receivedDataMutex;
        infra::Function<void(infra::ConstByteRange data)> receivedData;
    };
}

#endif
