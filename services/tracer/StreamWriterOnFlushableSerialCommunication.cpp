#include "services/tracer/StreamWriterOnFlushableSerialCommunication.hpp"

namespace services
{
    StreamWriterOnFlushableSerialCommunication::StreamWriterOnFlushableSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication, services::Flushable& flushable)
        : StreamWriterOnSerialCommunication(bufferStorage, communication)
        , flushable(flushable)
    {}

    void StreamWriterOnFlushableSerialCommunication::Flush()
    {
        while (currentlySendingBytes != 0)
        {
            flushable.Flush();
            OnCommunicationDone(transactionId);
            transactionId++;
        }
    }
}
