#include "services/tracer/StreamWriterOnSynchronousSerialCommunication.hpp"

namespace services
{
    StreamWriterOnSynchronousSerialCommunication::StreamWriterOnSynchronousSerialCommunication(hal::SynchronousSerialCommunication& communication)
        : communication(communication)
    {}

    void StreamWriterOnSynchronousSerialCommunication::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        communication.SendData(range);
    }

    std::size_t StreamWriterOnSynchronousSerialCommunication::Available() const
    {
        return std::numeric_limits<size_t>::max();
    }

}
