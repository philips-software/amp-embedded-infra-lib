#include "hal/unix/UartUnix.hpp"
#ifdef EMIL_OS_DARWIN
#include <IOKit/serial/ioss.h>
#else
#include <asm/termbits.h>
#endif
#include <fcntl.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

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
        speed_t bitrate = 115200;
        fileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

        if (fileDescriptor == -1)
            throw std::runtime_error(std::string("Could not open port (" + portName + "): ") + strerror(errno));

        if (ioctl(fileDescriptor, TIOCEXCL) == -1)
            throw std::runtime_error("Failed to get exclusive access to port (" + portName + ")");

        if (fcntl(fileDescriptor, F_SETFL, 0) == -1)
            throw std::runtime_error("Failed to clear O_NONBLOCK");

#ifdef EMIL_OS_DARWIN
        struct termios settings;
        if (tcgetattr(fileDescriptor, &settings) != 0)
            throw std::runtime_error("Could not get port configuration");
#else
        struct termios2 settings;
        if (ioctl(fileDescriptor, TCGETS2, &settings) == -1)
            throw std::runtime_error("Could not get port configuration");
#endif

        settings.c_cflag &= ~CSIZE;
        settings.c_cflag |= CS8;
        settings.c_cflag &= ~PARENB;
        settings.c_cflag &= ~CSTOPB;
        settings.c_cflag &= ~CRTSCTS;
        settings.c_cflag |= CREAD | CLOCAL;

#ifdef EMIL_OS_DARWIN
        bool darwinSetSpeed = true;
#else
        settings.c_cflag &= ~CBAUD;
        settings.c_cflag |= CBAUDEX;

        settings.c_ispeed = bitrate;
        settings.c_ospeed = bitrate;
#endif

        settings.c_oflag = 0;
        settings.c_oflag &= ~OPOST;

        settings.c_cc[VTIME] = 5000 / 100;
        settings.c_cc[VMIN] = 0;

        settings.c_iflag &= ~(IXON | IXOFF | IXANY);
        settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
        settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

#ifdef EMIL_OS_DARWIN
        if (tcsetattr(fileDescriptor, TCSANOW, &settings) != 0)
            throw std::runtime_error("Could not set port configuration");

        if (darwinSetSpeed)
        {
            if (ioctl(fileDescriptor, IOSSIOSPEED, &bitrate) == -1)
                throw std::runtime_error("Could not set custom baudrate");
        }
#else
        if (ioctl(fileDescriptor, TCSETS2, &settings) == -1)
            throw std::runtime_error("Could not set port configuration");
#endif

        readThread = std::thread([this]()
            { Read(); });
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
