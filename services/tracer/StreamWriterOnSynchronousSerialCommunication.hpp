#ifndef SERVICES_TRACER_STREAM_WRITER_ON_SYNCHRONOUS_SERIAL_COMMUNICATION_HPP
#define SERVICES_TRACER_STREAM_WRITER_ON_SYNCHRONOUS_SERIAL_COMMUNICATION_HPP

#include "hal/synchronous_interfaces/SynchronousSerialCommunication.hpp"
#include "infra/stream/OutputStream.hpp"

namespace services
{
    class StreamWriterOnSynchronousSerialCommunication
        : public infra::StreamWriter
    {
    public:
        explicit StreamWriterOnSynchronousSerialCommunication(hal::SynchronousSerialCommunication& communication);

        virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        virtual std::size_t Available() const override;

    private:
        hal::SynchronousSerialCommunication& communication;
    };
}

#endif
