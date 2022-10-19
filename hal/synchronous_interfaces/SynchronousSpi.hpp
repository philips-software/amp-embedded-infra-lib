#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_SPI_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_SPI_HPP

#include "infra/util/ByteRange.hpp"
#include <cstdint>

namespace hal
{
    class SynchronousSpi
    {
    public:
        SynchronousSpi() = default;
        SynchronousSpi(const SynchronousSpi& other) = delete;
        SynchronousSpi& operator=(const SynchronousSpi& other) = delete;

    protected:
        ~SynchronousSpi() = default;

    public:
        enum Action
        {
            continueSession,
            stop
        };

        void SendData(infra::ConstByteRange data, Action nextAction);
        void ReceiveData(infra::ByteRange data, Action nextAction);
        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, Action nextAction) = 0;
    };
}

#endif
