#include "hal/unix/UartUnix.hpp"
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

namespace hal
{
    UartUnix::UartUnix(const std::string& portName)
    {
        Open(portName);
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

        close(fileDescriptor);
    }

    void UartUnix::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        if (fileDescriptor < 0)
            throw std::runtime_error("Port not open");

        auto result = write(fileDescriptor, data.begin(), data.size());

        if (result == -1)
            throw std::system_error(EFAULT, std::system_category());
    }

    void UartUnix::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        std::unique_lock lock(mutex);
        receivedData = dataReceived;
        receivedDataSet.notify_all();
    }

    void UartUnix::Open(const std::string& portName)
    {
        fileDescriptor = open(portName.c_str(), O_RDWR);

        if (fileDescriptor == -1)
            throw std::runtime_error("Could not open port");

        struct termios2 settings;
        ioctl(fileDescriptor, TCGETS2, &settings);

        settings.c_cflag &= ~CSIZE;
        settings.c_cflag |= CS8;
        settings.c_cflag &= ~PARENB;
        settings.c_cflag &= ~CSTOPB;
        settings.c_cflag &= ~CRTSCTS;
        settings.c_cflag |= CREAD | CLOCAL;
        settings.c_cflag &= ~CBAUD;
        settings.c_cflag |= CBAUDEX;

        settings.c_ispeed = 115200;
        settings.c_ospeed = 115200;

        settings.c_oflag = 0;
        settings.c_oflag &= ~OPOST;

        settings.c_cc[VTIME] = 5000 / 100;
        settings.c_cc[VMIN] = 0;

        settings.c_iflag &= ~(IXON | IXOFF | IXANY);
        settings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

        settings.c_lflag &= ~ICANON;
        settings.c_lflag &= ~ECHO;

        settings.c_lflag &= ~ECHOE;
        settings.c_lflag &= ~ECHONL;
        settings.c_lflag &= ~ISIG;

        ioctl(fileDescriptor, TCSETS2, &settings);

        readThread = std::thread([this]() { Read(); });
    }

    void UartUnix::Read()
    {
        {
            std::unique_lock lock(mutex);
            if (!receivedData)
                receivedDataSet.wait(lock);
        }

        while (running)
        {
            auto range = infra::MakeByteRange(buffer);
            auto size = read(fileDescriptor, range.begin(), range.size());

            if (size < 0)
                throw std::system_error(EFAULT, std::system_category());

            std::unique_lock lock(mutex);
            receivedData(infra::ConstByteRange(range.begin(), range.begin() + size));
        }
    }
}
