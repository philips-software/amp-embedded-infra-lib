#include "protobuf/echo/test_helper/EchoOnStreamsHelper.hpp"
#include "infra/stream/BoundedVectorInputStream.hpp"

namespace application
{
    EchoOnStreamsHelper::EchoOnStreamsHelper(infra::BoundedVector<uint8_t>& storage)
        : storage(storage)
        , sendStream([this]() { SendStreamFilled(); })
    {}

    void EchoOnStreamsHelper::RequestSendStream(std::size_t size)
    {
        storage.clear();
        messageSize = size;
        auto writer = sendStream.Emplace(infra::inPlace, storage, messageSize);
        SetStreamWriter(std::move(writer));
    }

    void EchoOnStreamsHelper::BusyServiceDone()
    {
        // In this class, services are never busy, so BusyServiceDone() is never invoked
        std::abort();
    }

    void EchoOnStreamsHelper::SendStreamFilled()
    {
        infra::LimitedStreamReader::WithInput<infra::BoundedVectorInputStreamReader> reader(infra::inPlace, storage, messageSize);
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::softFail);
        if (!EchoOnStreams::ProcessMessage(stream))
            errorPolicy.MessageFormatError();
    }
}
