#include "hal/synchronous_interfaces/test_doubles/SynchronousSpiMock.hpp"

namespace hal
{
    void SynchronousSpiMock::SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, Action nextAction)
    {
        if (!sendData.empty())
            SendDataMock(std::vector<uint8_t>(sendData.begin(), sendData.end()), nextAction);

        if (!receiveData.empty())
        {
            std::vector<uint8_t> dataToBeReceived = ReceiveDataMock(nextAction);
            EXPECT_EQ(dataToBeReceived.size(), receiveData.size());
            std::copy(dataToBeReceived.begin(), dataToBeReceived.end(), receiveData.begin());
        }
    }
}
