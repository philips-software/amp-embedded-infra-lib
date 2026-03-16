#include "services/tracer/StreamWriterOnFlushableSerialCommunication.hpp"
#include "infra/util/MemoryRange.hpp"

namespace services
{
    StreamWriterOnFlushableSerialCommunication::StreamWriterOnFlushableSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication, hal::Flushable& flushableCommunication)
        : buffer(bufferStorage)
        , communication(communication)
        , flushableCommunication(flushableCommunication)
    {}

    void StreamWriterOnFlushableSerialCommunication::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        range.shrink_from_back_to(buffer.Available());
        buffer.Push(range);
        TrySend();
    }

    std::size_t StreamWriterOnFlushableSerialCommunication::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }

    void StreamWriterOnFlushableSerialCommunication::Flush()
    {
        while (currentlySendingBytes != 0 || !buffer.Empty())
        {
            flushableCommunication.Flush();
        }
    }

    void StreamWriterOnFlushableSerialCommunication::TrySend()
    {
        if (!buffer.Empty() && currentlySendingBytes == 0)
        {
            auto contiguousBytesToSend = buffer.ContiguousRange();
            currentlySendingBytes = contiguousBytesToSend.size();
            communication.SendData(contiguousBytesToSend, [this, completedTransactionId = ++transactionId]()
                {
                    CommunicationDone(completedTransactionId);
                });
        }
    }

    void StreamWriterOnFlushableSerialCommunication::CommunicationDone(uint16_t completedTransactionId)
    {
        if (completedTransactionId != transactionId)
            return;

        buffer.Pop(currentlySendingBytes);
        currentlySendingBytes = 0;

        TrySend();
    }
}
