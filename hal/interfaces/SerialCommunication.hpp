#ifndef HAL_SERIAL_COMMUNICATION_HPP
#define HAL_SERIAL_COMMUNICATION_HPP

#include "infra/event/AtomicTriggerScheduler.hpp"
#include "infra/stream/AtomicByteQueue.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"

namespace hal
{
    class SerialCommunication
    {
    protected:
        SerialCommunication() = default;
        ~SerialCommunication() = default;

    public:
        virtual void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) = 0;
        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) = 0;
    };

    class BufferedSerialCommunication;

    class BufferedSerialCommunicationObserver
        : public infra::SingleObserver<BufferedSerialCommunicationObserver, BufferedSerialCommunication>
    {
    public:
        using infra::SingleObserver<BufferedSerialCommunicationObserver, BufferedSerialCommunication>::SingleObserver;

        virtual void DataReceived() = 0;
    };

    class BufferedSerialCommunication
        : public infra::Subject<BufferedSerialCommunicationObserver>
    {
    public:
        virtual void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) = 0;

        virtual infra::StreamReaderWithRewinding& Reader() = 0;
        virtual void AckReceived() = 0;
    };

    class BufferedSerialCommunicationOnUnbuffered
        : public BufferedSerialCommunication
    {
    public:
        template<std::size_t Size>
        using WithStorage = infra::WithStorage<BufferedSerialCommunicationOnUnbuffered, infra::AtomicByteQueue::WithStorage<Size + 1>>;

        BufferedSerialCommunicationOnUnbuffered(infra::AtomicByteQueue& buffer, SerialCommunication& delegate);
        ~BufferedSerialCommunicationOnUnbuffered();

        // Implementation of BufferedSerialCommunication
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        infra::StreamReaderWithRewinding& Reader() override;
        void AckReceived() override;

    private:
        infra::AtomicByteQueue& buffer;
        SerialCommunication& delegate;

        infra::AtomicByteQueueReader reader{ buffer };
        infra::AtomicTriggerScheduler scheduler;
    };
}

#endif
