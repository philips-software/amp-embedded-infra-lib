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
        infra::SingleObserver<MessageCommunicationObserver, MessageCommunication>::SingleObserver;

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
        infra::SingleObserver<MessageCommunicationReceiveOnInterruptObserver, MessageCommunicationReceiveOnInterrupt>::SingleObserver;

        virtual void ReceivedMessageOnInterrupt(infra::StreamReader& reader) = 0;
    };

    class MessageCommunicationReceiveOnInterrupt
        : public infra::Subject<MessageCommunicationReceiveOnInterruptObserver>
    {
    public:
        virtual infra::SharedPtr<infra::StreamWriter> SendMessageStream(uint16_t size, const infra::Function<void(uint16_t size)>& onSent) = 0;
        virtual std::size_t MaxSendMessageSize() const = 0;
    };

    class WindowedMessageCommunication
        : public MessageCommunication
        , private MessageCommunicationReceiveOnInterruptObserver
    {
    public:
        template<std::size_t Size>
            using WithReceiveBuffer = infra::WithStorage<WindowedMessageCommunication, infra::BoundedDeque<uint8_t>::WithMaxSize<Size>>;

        WindowedMessageCommunication(infra::BoundedDeque<uint8_t>& receivedData, MessageCommunicationReceiveOnInterrupt& messageCommunication);

        // Implementation of MessageCommunication
        virtual void RequestSendMessage(uint16_t size) override;
        virtual std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of MessageCommunicationReceiveOnInterruptObserver
        virtual void ReceivedMessageOnInterrupt(infra::StreamReader& reader) override;

    private:
        void EvaluateReceiveMessage();
        void SetNextState(bool sendInitResponse);
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

        class State
        {
        public:
            virtual ~State() = default;

            virtual void RequestSendMessage(uint16_t size) = 0;

            virtual void ReceivedInit(uint16_t availableWindow) = 0;
            virtual void ReceivedInitResponse(uint16_t availableWindow) = 0;
            virtual void ReceivedReleaseWindow(uint16_t additionalAvailableWindow) = 0;
            virtual void ReceivedMessage(infra::StreamReader& reader) = 0;
        };

        class StateSendingInit
            : public State
        {
        public:
            StateSendingInit(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;
            virtual void ReceivedInit(uint16_t availableWindow) override;
            virtual void ReceivedInitResponse(uint16_t availableWindow) override;
            virtual void ReceivedReleaseWindow(uint16_t additionalAvailableWindow) override;
            virtual void ReceivedMessage(infra::StreamReader& reader) override;

        private:
            void OnSent();

        private:
            WindowedMessageCommunication& communication;
            bool sendInitResponse = false;
        };

        class StateReceivable
            : public State
        {
        public:
            StateReceivable(WindowedMessageCommunication& communication);

            virtual void ReceivedMessage(infra::StreamReader& reader) override;

        protected:
            void NextState();

        protected:
            WindowedMessageCommunication& communication;
            bool sendInitResponse = false;
        };

        class StateSendingInitResponse
            : public StateReceivable
        {
        public:
            StateSendingInitResponse(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;
            virtual void ReceivedInit(uint16_t availableWindow) override;
            virtual void ReceivedInitResponse(uint16_t availableWindow) override;
            virtual void ReceivedReleaseWindow(uint16_t additionalAvailableWindow) override;

        private:
            void OnSent();

        private:
            bool sendInitResponse = false;
        };

        class StateOperational
            : public StateReceivable
        {
        public:
            using StateReceivable::StateReceivable;

            virtual void RequestSendMessage(uint16_t size) override;
            virtual void ReceivedInit(uint16_t availableWindow) override;
            virtual void ReceivedInitResponse(uint16_t availableWindow) override;
            virtual void ReceivedReleaseWindow(uint16_t additionalAvailableWindow) override;
        };

        class StateSendingMessage
            : public StateReceivable
        {
        public:
            StateSendingMessage(WindowedMessageCommunication& communication);

            virtual void RequestSendMessage(uint16_t size) override;
            virtual void ReceivedInit(uint16_t availableWindow) override;
            virtual void ReceivedInitResponse(uint16_t availableWindow) override;
            virtual void ReceivedReleaseWindow(uint16_t additionalAvailableWindow) override;

        private:
            void OnSent(uint16_t sent);
        };

    private:
        infra::BoundedDeque<uint8_t>& receivedData;
        infra::NotifyingSharedOptional<infra::LimitedStreamReaderWithRewinding::WithInput<infra::BoundedDequeInputStreamReader>> reader;
        std::atomic<bool> notificationScheduled{ false };
        uint16_t otherAvailableWindow = 0;
        infra::Optional<uint16_t> requestedSendMessageSize;
        infra::PolymorphicVariant<State, StateSendingInit, StateSendingInitResponse, StateOperational, StateSendingMessage> state;
    };
}

#endif
