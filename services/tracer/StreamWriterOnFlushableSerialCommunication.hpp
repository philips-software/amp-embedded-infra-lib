#ifndef SERVICES_STREAM_WRITER_ON_FLUSHABLE_SERIAL_COMMUNICATION_HPP
#define SERVICES_STREAM_WRITER_ON_FLUSHABLE_SERIAL_COMMUNICATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/CyclicBuffer.hpp"

namespace services
{
    class StreamWriterOnFlushableSerialCommunication
        : public infra::StreamWriter
    {
    public:
        template<std::size_t StorageSize>
        using WithStorage = infra::WithStorage<StreamWriterOnFlushableSerialCommunication, std::array<uint8_t, StorageSize>>;

        StreamWriterOnFlushableSerialCommunication(infra::ByteRange bufferStorage, hal::SerialCommunication& communication, hal::Flushable& flushableCommunication);

        void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
        void Flush() override;

    private:
        void TrySend();
        void CommunicationDone(uint16_t completedTransactionId);

    private:
        infra::CyclicByteBuffer buffer;
        hal::SerialCommunication& communication;
        hal::Flushable& flushableCommunication;
        uint32_t currentlySendingBytes = 0;
        uint16_t transactionId = 0;
    };
}

#endif
