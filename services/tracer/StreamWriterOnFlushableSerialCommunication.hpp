#ifndef SERVICES_STREAM_WRITER_ON_FLUSHABLE_SERIAL_COMMUNICATION_HPP
#define SERVICES_STREAM_WRITER_ON_FLUSHABLE_SERIAL_COMMUNICATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/CyclicBuffer.hpp"
#include "services/util/Flushable.hpp"

namespace services
{
    class StreamWriterOnFlushableSerialCommunication
        : public infra::StreamWriter
        , public services::Flushable
    {
    public:
        template<std::size_t StorageSize>
        using WithStorage = infra::WithStorage<StreamWriterOnFlushableSerialCommunication, std::array<uint8_t, StorageSize>>;

        StreamWriterOnFlushableSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication, services::Flushable& flushableCommunication);

        // Implementation of infra::StreamWriter
        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;

        // Implementation of services::Flushable
        void Flush() override;

    private:
        void TrySend();
        void CommunicationDone(uint32_t completedTransactionId);

    private:
        infra::CyclicByteBuffer buffer;
        hal::SerialCommunication& communication;
        services::Flushable& flushableCommunication;
        uint32_t currentlySendingBytes = 0;
        uint32_t transactionId = 0;
    };
}

#endif
