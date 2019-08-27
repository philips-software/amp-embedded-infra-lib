#include "services/synchronous_util/SynchronousSpiMasterWithChipSelect.hpp"

namespace services
{
    SynchronousSpiMasterWithChipSelect::SynchronousSpiMasterWithChipSelect(hal::SynchronousSpi& spi, hal::GpioPin& chipSelect)
        : spi(spi)
        , chipSelect(chipSelect)
    {
        chipSelect.Set(true);
    }

    void SynchronousSpiMasterWithChipSelect::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, Action nextAction)
    {
        chipSelect.Set(false);
        spi.SendAndReceive(sendData, receiveData, nextAction);

        if (nextAction == Action::stop)
            chipSelect.Set(true);
    }

    SynchronousSpiMasterWithSynchronousChipSelect::SynchronousSpiMasterWithSynchronousChipSelect(
        hal::SynchronousSpi& spi, hal::SynchronousOutputPin& chipSelect)
        : spi(spi)
        , chipSelect(chipSelect)
    {
        chipSelect.Set(true);
    }

    void SynchronousSpiMasterWithSynchronousChipSelect::SendAndReceive(infra::ConstByteRange sendData,
        infra::ByteRange receiveData, Action nextAction)
    {
        chipSelect.Set(false);
        spi.SendAndReceive(sendData, receiveData, nextAction);

        if (nextAction == Action::stop)
            chipSelect.Set(true);
    }
}
