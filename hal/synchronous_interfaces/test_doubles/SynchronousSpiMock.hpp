#ifndef SYNCHRONOUS_SPI_MOCK_HPP
#define SYNCHRONOUS_SPI_MOCK_HPP

#include "hal/synchronous_interfaces/SynchronousSpi.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class SynchronousSpiMock
        : public hal::SynchronousSpi
    {
    public:
        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, Action nextAction) override;

        MOCK_METHOD2(SendDataMock, void(std::vector<uint8_t> dataSent, Action nextAction));
        MOCK_METHOD1(ReceiveDataMock, std::vector<uint8_t>(Action nextAction));
    };
}

#endif
