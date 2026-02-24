#include "services/tracer/StreamWriterOnSerialCommunication.hpp"
#include "infra/util/MemoryRange.hpp"

namespace services
{
    StreamWriterOnSerialCommunication::StreamWriterOnSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication)
        : buffer(bufferStorage)
        , communication(communication)
    {}

    void StreamWriterOnSerialCommunication::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        range.shrink_from_back_to(buffer.Available());
        buffer.Push(range);
        TrySend();
    }

    std::size_t StreamWriterOnSerialCommunication::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }

    void StreamWriterOnSerialCommunication::Flush()
    {
        auto remainingData = infra::DiscardHead(buffer.ContiguousRange(), currentlySendingBytes);
        size_t bytesFlushed = communication.SendDataBlocking(remainingData);
        CommunicationDone(bytesFlushed);
    }

    void StreamWriterOnSerialCommunication::TrySend()
    {
        if (!buffer.Empty() && currentlySendingBytes == 0)
        {
            currentlySendingBytes = buffer.ContiguousRange().size();
            communication.SendData(buffer.ContiguousRange(), [this]()
                {
                    CommunicationDone(currentlySendingBytes);
                });
        }
    }

    void StreamWriterOnSerialCommunication::CommunicationDone(uint32_t size)
    {
        buffer.Pop(size);
        currentlySendingBytes = 0;

        TrySend();
    }
}
