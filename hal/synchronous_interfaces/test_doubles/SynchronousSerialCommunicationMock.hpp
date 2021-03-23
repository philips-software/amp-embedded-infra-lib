#ifndef UPGRADE_SYNCHRONOUS_SERIAL_COMMUNICATION_MOCK_HPP
#define UPGRADE_SYNCHRONOUS_SERIAL_COMMUNICATION_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"

namespace hal
{
    class SynchronousSerialCommunicationMock
        : public SynchronousSerialCommunication
    {
    public:
        virtual void SendData(infra::ConstByteRange data) override;
        virtual bool ReceiveData(infra::ByteRange data) override;

        using ReceiveDataMockResult = std::pair<bool, std::vector<uint8_t>>;

        MOCK_METHOD1(SendDataMock, void(std::vector<uint8_t>));
        MOCK_METHOD0(ReceiveDataMock, ReceiveDataMockResult());
    };
}

#endif
