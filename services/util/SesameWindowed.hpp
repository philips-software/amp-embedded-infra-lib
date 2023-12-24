#ifndef SERVICES_MESSAGE_COMMUNICATION_WINDOWED_HPP
#define SERVICES_MESSAGE_COMMUNICATION_WINDOWED_HPP

#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/Aligned.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/util/Sesame.hpp"

namespace services
{
    class SesameWindowed
        : public Sesame
        , private SesameEncodedObserver
    {
    public:
        static constexpr uint32_t RawMessageSize(uint32_t messageSize)
        {
            return messageSize + sizeof(uint32_t);
        };

        SesameWindowed(SesameEncoded& delegate, uint16_t ownWindowSize);

        // Implementation of Sesame
        void RequestSendMessage(std::size_t size) override;
        std::size_t MaxSendMessageSize() const override;

    protected:
        virtual void ReceivedInit(uint16_t otherAvailableWindow)
        {}

        virtual void ReceivedInitResponse(uint16_t otherAvailableWindow)
        {}

        virtual void ReceivedReleaseWindow(uint16_t oldOtherAvailableWindow, uint16_t otherAvailableWindow)
        {}

        virtual void ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader)
        {}

    private:
        // Implementation of SesameEncodedObserver
        void Initialized() override;
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void MessageSent(std::size_t encodedSize) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

    private:
        void ForwardReceivedMessage();
        void SetNextState();

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
            explicit State(SesameWindowed& communication);
            virtual ~State() = default;

            virtual void Request();
            virtual void RequestSendMessage(std::size_t size);
            virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer);
            virtual void MessageSent(std::size_t encodedSize);

        protected:
            SesameWindowed& communication;
        };

        class StateSendingInit
            : public State
        {
        public:
            explicit StateSendingInit(SesameWindowed& communication);

            void Request() override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            void MessageSent(std::size_t encodedSize) override;
        };

        class StateSendingInitResponse
            : public State
        {
        public:
            explicit StateSendingInitResponse(SesameWindowed& communication);

            void Request() override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            void OnSent();
        };

        class StateOperational
            : public State
        {
        public:
            explicit StateOperational(SesameWindowed& communication);

            void RequestSendMessage(std::size_t size) override;
        };

        class StateSendingMessage
            : public State
        {
        public:
            explicit StateSendingMessage(SesameWindowed& communication);

            void Request() override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            void MessageSent(std::size_t encodedSize) override;

            void OnSent();

        private:
            std::size_t requestedSize;
            infra::SharedPtr<infra::StreamWriter> messageWriter;
            infra::AccessedBySharedPtr writerAccess{ [this]()
                {
                    OnSent();
                } };
        };

        class StateSendingReleaseWindow
            : public State
        {
        public:
            explicit StateSendingReleaseWindow(SesameWindowed& communication);

            void Request() override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            void OnSent();
        };

    private:
        uint16_t releaseWindowSize;
        uint16_t ownWindowSize;
        bool initialized = false;
        infra::SharedPtr<infra::StreamReaderWithRewinding> receivedMessageReader;
        infra::AccessedBySharedPtr readerAccess;
        uint16_t otherAvailableWindow{ 0 };
        uint16_t releasedWindow{ 0 };
        bool sendInitResponse{ false };
        bool sending = false;
        infra::Optional<std::size_t> requestedSendMessageSize;
        infra::PolymorphicVariant<State, StateSendingInit, StateSendingInitResponse, StateOperational, StateSendingMessage, StateSendingReleaseWindow> state;
    };
}

#endif
