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
        while (currentlySendingBytes != 0 || !buffer.Empty())
        {
            communication.FlushSendBuffer();
            if (currentlySendingBytes != 0)
            {
                // The flush failed, likely not supported by communication. Continue without attempting flush
                break;
            }
        }
    }

    void StreamWriterOnSerialCommunication::TrySend()
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

    void StreamWriterOnSerialCommunication::CommunicationDone(uint16_t completedTransactionId)
    {
        if (completedTransactionId != transactionId)
            return;

        buffer.Pop(currentlySendingBytes);
        currentlySendingBytes = 0;

        TrySend();
    }
}
