#include "services/tracer/StreamWriterOnSerialCommunication.hpp"

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

    void StreamWriterOnSerialCommunication::TrySend()
    {
        if (!buffer.Empty() && !communicating)
        {
            communicating = true;

            uint32_t size = buffer.ContiguousRange().size();
            communication.SendData(buffer.ContiguousRange(), [this, size]()
                { CommunicationDone(size); });
        }
    }

    void StreamWriterOnSerialCommunication::CommunicationDone(uint32_t size)
    {
        communicating = false;
        buffer.Pop(size);

        TrySend();
    }
}
