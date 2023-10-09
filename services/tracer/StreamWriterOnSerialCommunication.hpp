#ifndef SERVICES_STREAM_WRITER_ON_SERIAL_COMMUNICATION_HPP
#define SERVICES_STREAM_WRITER_ON_SERIAL_COMMUNICATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/CyclicBuffer.hpp"

namespace services
{
    class StreamWriterOnSerialCommunication
        : public infra::StreamWriter
    {
    public:
        template<std::size_t StorageSize>
        using WithStorage = infra::WithStorage<StreamWriterOnSerialCommunication, std::array<uint8_t, StorageSize>>;

        StreamWriterOnSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication);

        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;

    private:
        void TrySend();
        void CommunicationDone(uint32_t size);

    private:
        infra::CyclicByteBuffer buffer;
        hal::SerialCommunication& communication;
        bool communicating = false;
    };
}

#endif
