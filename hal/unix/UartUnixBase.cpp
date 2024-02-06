#include "hal/unix/UartUnixBase.hpp"
#include "infra/event/EventDispatcher.hpp"
#include <iomanip>
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
    PortNotOpened::PortNotOpened(const std::string& portName, const std::string& errstr)
        : std::runtime_error("Unable to open port \"" + portName + "\" : \"" + errstr + "\"")
    {}

    UartUnixBase::UartUnixBase(const std::string& portName, const Config& config)
    {
        Open(portName, config.baudrate, config.flowControl);
    }

    UartUnixBase::~UartUnixBase()
    {
        close(fileDescriptor);
    }

    void UartUnixBase::Open(const std::string& portName, uint32_t baudrate, bool flowControl)
    {
        speed_t bitrate = baudrate;
        fileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

        if (fileDescriptor == -1)
            throw PortNotOpened(portName, strerror(errno));

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

        if (flowControl)
            settings.c_cflag |= CRTSCTS;
        else
            settings.c_cflag &= ~CRTSCTS;

        settings.c_cflag |= CREAD | CLOCAL;

#ifndef EMIL_OS_DARWIN
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

        if (ioctl(fileDescriptor, IOSSIOSPEED, &bitrate) == -1)
            throw std::runtime_error("Could not set custom baudrate");
#else
        if (ioctl(fileDescriptor, TCSETS2, &settings) == -1)
            throw std::runtime_error("Could not set port configuration");
#endif
    }

    int UartUnixBase::FileDescriptor() const
    {
        return fileDescriptor;
    }
}
