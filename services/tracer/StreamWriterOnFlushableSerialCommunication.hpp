#ifndef SERVICES_STREAM_WRITER_ON_FLUSHABLE_SERIAL_COMMUNICATION_HPP
#define SERVICES_STREAM_WRITER_ON_FLUSHABLE_SERIAL_COMMUNICATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "services/tracer/StreamWriterOnSerialCommunication.hpp"
#include "services/util/Flushable.hpp"

namespace services
{
    class StreamWriterOnFlushableSerialCommunication
        : public services::StreamWriterOnSerialCommunication
        , public services::Flushable
    {
    public:
        template<std::size_t StorageSize>
        using WithStorage = infra::WithStorage<StreamWriterOnFlushableSerialCommunication, std::array<uint8_t, StorageSize>>;

        StreamWriterOnFlushableSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication, services::Flushable& flushable);

        // Implementation of services::Flushable
        void Flush() override;

    private:
        services::Flushable& flushable;
    };
}

#endif
