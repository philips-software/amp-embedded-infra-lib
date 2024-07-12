#include "hal/unix/UartUnix.hpp"
#include "hal/unix/UartUnixBase.hpp"
#include "infra/event/EventDispatcher.hpp"
#ifdef EMIL_OS_DARWIN
#include <IOKit/serial/ioss.h>
#else
#include <asm/termbits.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace hal
{

    UartUnix::UartUnix(const std::string& portName, const Config& config)
        : UartUnixBase(portName, config)
        , readThread{ [this]()
            {
                Read();
            } }
    {
    }

    UartUnix::~UartUnix()
    {
        {
            std::unique_lock lock(mutex);
            running = false;
            receivedDataSet.notify_all();
        }

        if (readThread.joinable())
            readThread.join();
    }

    void UartUnix::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        if (FileDescriptor() < 0)
            throw std::runtime_error("Port not open");

        auto result = write(FileDescriptor(), data.begin(), data.size());

        if (result == -1)
            throw std::system_error(EFAULT, std::system_category());

        infra::EventDispatcher::Instance().Schedule(actionOnCompletion);
    }

    void UartUnix::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        std::unique_lock lock(mutex);
        receivedData = dataReceived;
        receivedDataSet.notify_all();
    }

    void UartUnix::Read()
    {
        {
            std::unique_lock lock(mutex);
            receivedDataSet.wait(lock, [this]
                {
                    return static_cast<bool>(receivedData);
                });
        }

        while (running)
        {
            auto range = infra::MakeByteRange(buffer);
            auto size = read(FileDescriptor(), range.begin(), range.size());

            if (size < 0)
                throw std::system_error(EFAULT, std::system_category());

            std::unique_lock lock(mutex);
            if (receivedData)
                receivedData(infra::ConstByteRange(range.begin(), range.begin() + size));
        }
    }
}
