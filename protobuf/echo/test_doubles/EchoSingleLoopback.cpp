#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"

namespace application
{
    void EchoSingleLoopback::RequestSendStream(std::size_t size)
    {
        storage.clear();
        storage.resize(size);
        SendStreamAvailable(sendStream.Emplace(infra::MakeRange(storage)));
    }

    void EchoSingleLoopback::SendStreamFilled()
    {
        DataReceived(reader.Emplace(infra::MakeRange(storage)));
    }
}
