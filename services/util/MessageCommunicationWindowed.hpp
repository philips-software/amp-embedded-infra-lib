#ifndef SERVICES_MESSAGE_COMMUNICATION_WINDOWED_HPP
#define SERVICES_MESSAGE_COMMUNICATION_WINDOWED_HPP

#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/Aligned.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class MessageCommunicationWindowed
        : public MessageCommunication
        , private MessageCommunicationObserver
    {
    public:
        static constexpr uint32_t RawMessageSize(uint32_t messageSize)
        {
            return messageSize + sizeof(uint32_t);
        };

        MessageCommunicationWindowed(MessageCommunication& delegate, uint16_t ownWindowSize);

        // Implementation of MessageCommunication
        void RequestSendMessage(uint16_t size) override;
        std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of MessageCommunicationReceiveObserver
        void Initialized() override;
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

    private:
        void EvaluateReceiveMessage();
        void SetNextState();
        uint16_t WindowSize(uint16_t messageSize) const;

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

            virtual void Request();
            virtual void RequestSendMessage(uint16_t size) = 0;
            virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer);
        };

        class StateSendingInit
            : public State
        {
        public:
            explicit StateSendingInit(MessageCommunicationWindowed& communication);

            void Request() override;
            void RequestSendMessage(uint16_t size) override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            MessageCommunicationWindowed& communication;
        };

        class StateSendingInitResponse
            : public State
        {
        public:
            explicit StateSendingInitResponse(MessageCommunicationWindowed& communication);

            void Request() override;
            void RequestSendMessage(uint16_t size) override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

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

            void Request() override;
            void RequestSendMessage(uint16_t size) override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            void OnSent();

        private:
            MessageCommunicationWindowed& communication;
            uint16_t requestedSize;
            infra::SharedPtr<infra::StreamWriter> writer;
            infra::AccessedBySharedPtr writerAccess{ [this]()
                {
                    OnSent();
                } };
        };

        class StateSendingReleaseWindow
            : public State
        {
        public:
            explicit StateSendingReleaseWindow(MessageCommunicationWindowed& communication);

            void Request() override;
            void RequestSendMessage(uint16_t size) override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            void OnSent();

        private:
            MessageCommunicationWindowed& communication;
        };

    private:
        uint16_t ownWindowSize;
        bool initialized = false;
        infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
        infra::AccessedBySharedPtr readerAccess;
        uint16_t otherAvailableWindow{ 0 };
        uint16_t releasedWindow{ 0 };
        bool sendInitResponse{ false };
        bool sending = false;
        infra::Optional<uint16_t> requestedSendMessageSize;
        infra::PolymorphicVariant<State, StateSendingInit, StateSendingInitResponse, StateOperational, StateSendingMessage, StateSendingReleaseWindow> state;
    };
}

#endif
