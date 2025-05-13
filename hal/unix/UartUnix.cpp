#include "hal/unix/UartUnix.hpp"
#include "hal/unix/UartUnixBase.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include <cerrno>
#include <mutex>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
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
    {
    }

    UartUnix::~UartUnix()
    {
        {
            std::unique_lock lock(receivedDataMutex);
            running = false;
            receivedData = nullptr;
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
        running = true;

        {
            std::unique_lock lock(receivedDataMutex);
            receivedData = dataReceived;
        }

        if (!readThread.joinable())
            readThread = std::thread([this]
                {
                    Read();
                });
    }

    void UartUnix::Read()
    {
        while (running)
        {
            {
                std::unique_lock lock(receivedDataMutex);
                if (!running)
                    break;
            }
            auto range = infra::MakeByteRange(buffer);
            auto size = read(FileDescriptor(), range.begin(), range.size());

            if (size < 0)
                throw std::system_error(EFAULT, std::system_category());

            {
                std::unique_lock lock(receivedDataMutex);
                if (running && receivedData)
                    receivedData(infra::ConstByteRange(range.begin(), range.begin() + size));
            }
        }
    }
}
