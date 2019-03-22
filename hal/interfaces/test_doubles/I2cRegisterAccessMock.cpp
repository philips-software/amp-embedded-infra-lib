#include "hal/interfaces/test_doubles/I2cRegisterAccessMock.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/ByteInputStream.hpp"

namespace hal
{
    void I2cMasterRegisterAccessMock::SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent)
    {
        assert(atStart || sending);

        infra::ByteInputStream stream(data);

        if (atStart)
            stream >> dataRegister;

        if (!stream.Empty())
            currentSendData.insert(currentSendData.end(), stream.Reader().Remaining().begin(), stream.Reader().Remaining().end());

        if (nextAction != hal::Action::continueSession && !currentSendData.empty())
        {
            WriteRegisterMock(dataRegister, currentSendData);
            currentSendData.clear();
        }

        atStart = nextAction != hal::Action::continueSession;
        sending = true;
        onSent(hal::Result::complete, data.size());
    }

    void I2cMasterRegisterAccessMock::ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
        infra::Function<void(hal::Result)> onReceived)
    {
        assert(atStart || !sending);

        if (atStart)
            currentReceiveData = ReadRegisterMock(dataRegister);

        assert(currentReceiveData.size() >= data.size());
        std::copy(currentReceiveData.begin(), currentReceiveData.begin() + data.size(), data.begin());
        currentReceiveData.erase(currentReceiveData.begin(), currentReceiveData.begin() + data.size());

        atStart = nextAction != hal::Action::continueSession;
        sending = false;
        onReceived(hal::Result::complete);
    }
}
