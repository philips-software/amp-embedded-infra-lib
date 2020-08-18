#ifndef SERVICES_MESSAGE_COMMUNICATION_HPP
#define SERVICES_MESSAGE_COMMUNICATION_HPP

#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/Aligned.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include <atomic>

namespace services
{
    class MessageCommunication;

    class MessageCommunicationObserver
        : public infra::SingleObserver<MessageCommunicationObserver, MessageCommunication>
    {
    public:
        using infra::SingleObserver<MessageCommunicationObserver, MessageCommunication>::SingleObserver;

        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) = 0;
    };

    class MessageCommunication
        : public infra::Subject<MessageCommunicationObserver>
    {
    public:
        virtual void RequestSendMessage(uint16_t size) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
    };

    class MessageCommunicationReceiveOnInterrupt;

    class MessageCommunicationReceiveOnInterruptObserver
        : public infra::SingleObserver<MessageCommunicationReceiveOnInterruptObserver, MessageCommunicationReceiveOnInterrupt>
    {
    public:
        using infra::SingleObserver<MessageCommunicationReceiveOnInterruptObserver, MessageCommunicationReceiveOnInterrupt>::SingleObserver;

        virtual void ReceivedMessageOnInterrupt(infra::StreamReader& reader) = 0;
    };

    class MessageCommunicationReceiveOnInterrupt
        : public infra::Subject<MessageCommunicationReceiveOnInterruptObserver>
    {
    public:
        virtual infra::SharedPtr<infra::StreamWriter> SendMessageStream(uint16_t size, const infra::Function<void(uint16_t size)>& onSent) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
    };

    namespace detail
    {
        class AtomicDeque
        {
        public:
            template<std::size_t Size>
                using WithStorage = infra::WithStorage<AtomicDeque, std::array<uint8_t, Size + 1>>;

            AtomicDeque(infra::ByteRange storage);

            void Push(infra::ConstByteRange range);
            void Pop(std::size_t size);

            std::size_t Size() const;
            std::size_t MaxSize() const;
            bool Empty() const;

            infra::ConstByteRange PeekContiguousRange(std::size_t start) const;

        public:
            infra::ByteRange storage;
            std::atomic<uint8_t*> b{ storage.begin() };
            std::atomic<uint8_t*> e{ storage.begin() };
        };

        class AtomicDequeReader
            : public infra::StreamReaderWithRewinding
        {
        public:
            AtomicDequeReader(const AtomicDeque& deque);

            virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            virtual bool Empty() const override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual void Rewind(std::size_t marker) override;

        private:
            const AtomicDeque& deque;
            std::size_t offset = 0;
        };

        class AtomicDequeWriter
            : public infra::StreamWriter
        {
        public:
            AtomicDequeWriter(AtomicDeque& deque);

            virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual std::size_t GetProcessedBytesSince(std::size_t marker) const;
            virtual infra::ByteRange SaveState(std::size_t marker) override;
            virtual void RestoreState(infra::ByteRange range) override;
            virtual infra::ByteRange Overwrite(std::size_t marker) override;

        private:
            AtomicDeque& deque;
        };
    }

    class WindowedMessageCommunication
        : public MessageCommunication
        , private MessageCommunicationReceiveOnInterruptObserver
    {
    public:
        template<std::size_t Size>
            using WithReceiveBuffer = infra::WithStorage<WindowedMessageCommunication, detail::AtomicDeque::WithStorage<Size>>;

        WindowedMessageCommunication(detail::AtomicDeque& receivedData, MessageCommunicationReceiveOnInterrupt& messageCommunication);

        // Implementation of MessageCommunication
        virtual void RequestSendMessage(uint16_t size) override;
        virtual std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of MessageCommunicationReceiveOnInterruptObserver
        virtual void ReceivedMessageOnInterrupt(infra::StreamReader& reader) override;

    private:
        void ReceivedMessage(infra::StreamReader& reader);
        bool EvaluateReceiveMessage();
        void SetNextState();
        uint16_t WindowSize(uint16_t messageSize) const;
        uint16_t AvailableWindow() const;

    private:
        enum class Operation
            : uint8_t
        {
            init = 1,
            initResponse,
            releaseWindow,
            message
        };

        struct PacketInit
        {
            PacketInit(uint16_t window);

            Operation operation = Operation::init;
            infra::Aligned<uint8_t, infra::LittleEndian<uint16_t>> window;
        };

        struct PacketInitResponse
        {
            PacketInitResponse(uint16_t window);

            Operation operation = Operation::initResponse;
            infra::Aligned<uint8_t, infra::LittleEndian<uint16_t>> window;
        };

        struct PacketReleaseWindow
        {
            PacketReleaseWindow(uint16_t window);

            Operation operation = Operation::releaseWindow;
            infra::Aligned<uint8_t, infra::LittleEndian<uint16_t>> window;
        };

        class State
        {
        public:
            virtual ~State() = default;

            virtual void RequestSendMessage(uint16_t size) = 0;
        };

        class StateSendingInit
            : public State
        {
        public:
            StateSendingInit(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent();

        private:
            WindowedMessageCommunication& communication;
        };

        class StateSendingInitResponse
            : public State
        {
        public:
            StateSendingInitResponse(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent();

        private:
            WindowedMessageCommunication& communication;
        };

        class StateOperational
            : public State
        {
        public:
            StateOperational(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;

        private:
            WindowedMessageCommunication& communication;
        };

        class StateSendingMessage
            : public State
        {
        public:
            StateSendingMessage(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent(uint16_t sent);

        private:
            WindowedMessageCommunication& communication;
        };

        class StateSendingReleaseWindow
            : public State
        {
        public:
            StateSendingReleaseWindow(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent();

        private:
            WindowedMessageCommunication& communication;
        };

    private:
        detail::AtomicDeque& receivedData;
        bool initialized = false;
        infra::NotifyingSharedOptional<infra::LimitedStreamReaderWithRewinding::WithInput<detail::AtomicDequeReader>> reader;
        std::atomic<bool> notificationScheduled{ false };
        std::atomic<uint16_t> otherAvailableWindow{ 0 };
        std::atomic<bool> switchingState{ false };
        uint16_t releasedWindowBuffer = 0;
        std::atomic<uint16_t> releasedWindow{ 0 };
        bool sendInitResponse = false;
        bool sending = false;
        infra::Optional<uint16_t> requestedSendMessageSize;
        infra::PolymorphicVariant<State, StateSendingInit, StateSendingInitResponse, StateOperational, StateSendingMessage, StateSendingReleaseWindow> state;
    };
}

#endif
