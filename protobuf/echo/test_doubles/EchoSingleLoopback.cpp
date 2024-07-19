#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"

namespace application
{
    void EchoSingleLoopback::RequestSendStream(std::size_t size)
    {
        storage.clear();
        storage.resize(size);
        infra::ReConstruct(sendStream, infra::MakeRange(storage));
        SendStreamAvailable(sendStreamAccess.MakeShared(sendStream));
    }

    EchoSingleLoopback::~EchoSingleLoopback()
    {
        ReleaseReader();
    }

    void EchoSingleLoopback::SendStreamFilled()
    {
        sending.push_back({ sendStream.Processed().begin(), sendStream.Processed().end() });

        TryForward();
    }

    void EchoSingleLoopback::TryForward()
    {
        if (reader.Allocatable() && !sending.empty())
        {
            auto data = sending.front();
            sending.pop_front();
            DataReceived(reader.Emplace(infra::MakeRange(data)));
        }
    }

    void EchoSingleLoopback::ReaderDone()
    {
        TryForward();
    }
}
