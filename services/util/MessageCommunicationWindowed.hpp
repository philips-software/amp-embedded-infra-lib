#ifndef SERVICES_MESSAGE_COMMUNICATION_WINDOWED_HPP
#define SERVICES_MESSAGE_COMMUNICATION_WINDOWED_HPP

#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/Aligned.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/util/MessageCommunication.hpp"
#include <atomic>

namespace services
{
    namespace detail
    {
        class AtomicDeque
        {
        public:
            template<std::size_t Size>
            using WithStorage = infra::WithStorage<AtomicDeque, std::array<uint8_t, Size + 1>>;

            explicit AtomicDeque(infra::ByteRange storage);

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
            explicit AtomicDequeReader(const AtomicDeque& deque);

            void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            bool Empty() const override;
            std::size_t Available() const override;
            std::size_t ConstructSaveMarker() const override;
            void Rewind(std::size_t marker) override;

        private:
            const AtomicDeque& deque;
            std::size_t offset = 0;
        };

        class AtomicDequeWriter
            : public infra::StreamWriter
        {
        public:
            explicit AtomicDequeWriter(AtomicDeque& deque);

            void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            std::size_t Available() const override;
            std::size_t ConstructSaveMarker() const override;
            std::size_t GetProcessedBytesSince(std::size_t marker) const override;
            infra::ByteRange SaveState(std::size_t marker) override;
            void RestoreState(infra::ByteRange range) override;
            infra::ByteRange Overwrite(std::size_t marker) override;

        private:
            AtomicDeque& deque;
        };
    }

    class MessageCommunicationWindowed
        : public MessageCommunication
        , private MessageCommunicationReceiveOnInterruptObserver
    {
    public:
        static constexpr uint32_t RawMessageSize(uint32_t messageSize)
        {
            return messageSize + sizeof(uint32_t);
        };

        template<std::size_t Size>
        using WithReceiveBuffer = infra::WithStorage<MessageCommunicationWindowed, detail::AtomicDeque::WithStorage<RawMessageSize(Size)>>;

        MessageCommunicationWindowed(detail::AtomicDeque& receivedData, MessageCommunicationReceiveOnInterrupt& messageCommunication);

        // Implementation of MessageCommunication
        void RequestSendMessage(uint16_t size) override;
        std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of MessageCommunicationReceiveOnInterruptObserver
        void ReceivedMessageOnInterrupt(infra::StreamReader& reader) override;

    private:
        void ReceivedMessage(infra::StreamReader& reader);
        bool EvaluateReceiveMessage();
        void SetNextState();
        uint16_t WindowSize(uint16_t messageSize) const;
        uint16_t AvailableWindow() const;

    private:
        enum class Operation : uint8_t
        {
            init = 1,
            initResponse,
            releaseWindow,
            message
        };

        struct PacketInit
        {
            explicit PacketInit(uint16_t window);

            Operation operation = Operation::init;
            infra::Aligned<uint8_t, infra::LittleEndian<uint16_t>> window;
        };

        struct PacketInitResponse
        {
            explicit PacketInitResponse(uint16_t window);

            Operation operation = Operation::initResponse;
            infra::Aligned<uint8_t, infra::LittleEndian<uint16_t>> window;
        };

        struct PacketReleaseWindow
        {
            explicit PacketReleaseWindow(uint16_t window);

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
            explicit StateSendingInit(MessageCommunicationWindowed& communication);

            void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent();

        private:
            MessageCommunicationWindowed& communication;
        };

        class StateSendingInitResponse
            : public State
        {
        public:
            explicit StateSendingInitResponse(MessageCommunicationWindowed& communication);

            void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent();

        private:
            MessageCommunicationWindowed& communication;
        };

        class StateOperational
            : public State
        {
        public:
            explicit StateOperational(MessageCommunicationWindowed& communication);

            void RequestSendMessage(uint16_t size) override;

        private:
            MessageCommunicationWindowed& communication;
        };

        class StateSendingMessage
            : public State
        {
        public:
            explicit StateSendingMessage(MessageCommunicationWindowed& communication);

            void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent(uint16_t sent);

        private:
            MessageCommunicationWindowed& communication;
        };

        class StateSendingReleaseWindow
            : public State
        {
        public:
            explicit StateSendingReleaseWindow(MessageCommunicationWindowed& communication);

            void RequestSendMessage(uint16_t size) override;

        private:
            void OnSent();

        private:
            MessageCommunicationWindowed& communication;
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
        std::atomic<bool> sendInitResponse{ false };
        bool sending = false;
        std::optional<uint16_t> requestedSendMessageSize;
        infra::PolymorphicVariant<State, StateSendingInit, StateSendingInitResponse, StateOperational, StateSendingMessage, StateSendingReleaseWindow> state;
    };
}

#endif
