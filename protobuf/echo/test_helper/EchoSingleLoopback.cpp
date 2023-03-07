#include "protobuf/echo/test_helper/EchoSingleLoopback.hpp"
#include "infra/stream/ByteInputStream.hpp"

namespace application
{
    void EchoSingleLoopback::RequestSendStream(std::size_t size)
    {
        storage.clear();
        storage.resize(size);
        auto writer = sendStream.Emplace(infra::MakeRange(storage));
        SetStreamWriter(std::move(writer));
    }

    void EchoSingleLoopback::BusyServiceDone()
    {
        // In this class, services are never busy, so BusyServiceDone() is never invoked
        std::abort();
    }

    void EchoSingleLoopback::SendStreamFilled()
    {
        infra::ByteInputStreamReader reader(infra::MakeRange(storage));
        if (!ProcessMessage(reader))
            errorPolicy.MessageFormatError();
    }
}
