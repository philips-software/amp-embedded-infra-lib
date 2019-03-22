#include "hal/interfaces/Spi.hpp"

namespace hal
{
    void SpiMaster::SendData(infra::ConstByteRange data, SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        SendAndReceive(data, infra::ByteRange(), nextAction, onDone);
    }

    void SpiMaster::ReceiveData(infra::ByteRange data, SpiAction nextAction, const infra::Function<void()>& onDone)
    {
        SendAndReceive(infra::ConstByteRange(), data, nextAction, onDone);
    }
}
