#include "hal/unix/SynchronousUartUnix.hpp"
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
    void SynchronousUartUnix::SendData(infra::ConstByteRange data)
    {
        if (FileDescriptor() < 0)
            throw std::runtime_error("Port not open");

        auto result = write(FileDescriptor(), data.begin(), data.size());

        if (result == -1)
            throw std::system_error(EFAULT, std::system_category());
    }

    bool SynchronousUartUnix::ReceiveData(infra::ByteRange data)
    {
        auto size = read(FileDescriptor(), data.begin(), data.size());
        return data.size() == size;
    }
}
