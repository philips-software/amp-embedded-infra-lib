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
        explicit SesameWindowed(SesameEncoded& delegate);

        void Stop();

        // Implementation of Sesame
        void RequestSendMessage(std::size_t size) override;
        std::size_t MaxSendMessageSize() const override;

    protected:
        virtual void ReceivedInit(uint16_t newWindow)
        {}

        virtual void ReceivedInitResponse(uint16_t newWindow)
        {}

        virtual void ReceivedReleaseWindow(uint16_t oldWindow, uint16_t newWindow)
        {}

        virtual void ForwardingReceivedMessage(infra::StreamReaderWithRewinding& reader)
        {}

        virtual void SendingInit(uint16_t newWindow)
        {}

        virtual void SendingInitResponse(uint16_t newWindow)
        {}

        virtual void SendingReleaseWindow(uint16_t deltaWindow)
        {}

        virtual void SendingMessage(infra::StreamWriter& writer)
        {}

        virtual void SettingOperational(infra::Optional<std::size_t> requestedSize, uint16_t releasedWindow, uint16_t otherWindow)
        {}

    private:
        // Implementation of SesameEncodedObserver
        void Initialized() override;
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void MessageSent(std::size_t encodedSize) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, std::size_t encodedSize) override;

    private:
        void ReceivedInitialize(uint16_t window);
        void ForwardReceivedMessage(uint16_t encodedSize);
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

        private:
            std::size_t requestedSize;
        };

        class StateSendingReleaseWindow
            : public State
        {
        public:
            explicit StateSendingReleaseWindow(SesameWindowed& communication);

            void Request() override;
            void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        };

    private:
        uint16_t ownBufferSize;
        uint16_t releaseWindowSize;
        bool initialized = false;
        infra::SharedPtr<infra::StreamReaderWithRewinding> receivedMessageReader;
        infra::AccessedBySharedPtr readerAccess;
        uint16_t otherAvailableWindow{ 0 };
        uint16_t maxUsableBufferSize = 0;
        uint16_t releasedWindow{ 0 };
        bool sendInitResponse{ false };
        bool sending = false;
        infra::Optional<std::size_t> requestedSendMessageSize;
        infra::PolymorphicVariant<State, StateSendingInit, StateSendingInitResponse, StateOperational, StateSendingMessage, StateSendingReleaseWindow> state;
    };
}

#endif
