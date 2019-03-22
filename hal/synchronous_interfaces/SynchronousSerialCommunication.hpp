#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_SERIAL_COMMUNICATION_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_SERIAL_COMMUNICATION_HPP

#include "infra/util/ByteRange.hpp"
#include <cstdint>

namespace hal
{
    class SynchronousSerialCommunication
    {
    public:
        SynchronousSerialCommunication() = default;
        SynchronousSerialCommunication(const SynchronousSerialCommunication& other) = delete;
        SynchronousSerialCommunication& operator=(const SynchronousSerialCommunication& other) = delete;

    protected:
        ~SynchronousSerialCommunication() = default;

    public:
        virtual void SendData(infra::ConstByteRange data) = 0;
        virtual bool ReceiveData(infra::ByteRange data) = 0;
    };
}

#endif
