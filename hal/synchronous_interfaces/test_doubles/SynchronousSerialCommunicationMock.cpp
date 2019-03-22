#include "hal/synchronous_interfaces/test_doubles/SynchronousSerialCommunicationMock.hpp"

namespace hal
{
    void SynchronousSerialCommunicationMock::SendData(infra::ConstByteRange data)
    {
        SendDataMock(std::vector<uint8_t>(data.begin(), data.end()));
    }

    bool SynchronousSerialCommunicationMock::ReceiveData(infra::ByteRange data)
    {
        std::pair<bool, std::vector<uint8_t>> result = ReceiveDataMock();
        if (!result.first)
            return false;

        assert(result.second.size() == data.size());
        std::copy(result.second.begin(), result.second.end(), data.begin());
        return true;
    }
}
