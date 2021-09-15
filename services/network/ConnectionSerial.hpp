#ifndef SERVICES_CONNECTION_SERIAL_HPP
#define SERVICES_CONNECTION_SERIAL_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class ConnectionSerial
        : public Connection
    {
    private:
        class MessageHeader;

    public:
        template<std::size_t Max>
        using WithStorage = infra::WithStorage<infra::WithStorage<infra::WithStorage<ConnectionSerial, std::array<uint8_t, Max>>, std::array<uint8_t, Max>>, infra::BoundedDeque<uint8_t>::WithMaxSize<Max>>;

        ConnectionSerial(infra::ByteRange sendBuffer, infra::ByteRange parseBuffer, infra::BoundedDeque<uint8_t>& receivedDataQueue, hal::SerialCommunication& serialCommunication, infra::Function<void()> onConnected, infra::Function<void()> onDisconnected, size_t minUpdateSize = MessageHeader::HeaderSize + 1);
        ConnectionSerial(const ConnectionSerial& other) = delete;
        ConnectionSerial& operator=(const ConnectionSerial& other) = delete;

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;

        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

    private:
        void TryProcessHeader();
        void TryConstructHeader();
        void ProcessHeader();
        void SendStreamFilled(infra::ByteRange data);
        void DataReceived();
        void ResetConnection();
        void SendStreamAllocatable();
        void InitCompleted();
        void AllocatedSendStream();
        void NotifyObserverDataReceived();
        size_t ReceiveBufferSize() const;

        void GoStateInitSizeRequest();
        void GoStateInitSizeResponseRequest();
        void GoStateInitSizeResponse();
        void GoStateConnectedIdle();
        void GoStateConnectedSendingContent(size_t size);
        void GoStateSendingUpdate();

    private:
        enum class MessageType : uint8_t
        {
            Escape = 0xf9, // f9-fe in data are escaped
            SizeRequest = 0xfa,
            SizeResponse = 0xfb,
            SizeResponseRequest = 0xfc,
            Content = 0xfd,
            SizeUpdate = 0xfe
        };

        class MessageHeader
        {
        public:
            MessageHeader();
            MessageHeader(MessageType type);

            void AddByte(uint8_t byte);
            void SetSize(size_t size);

            bool IsFailed();
            bool IsDone();

            MessageType Type();
            size_t Size();
            infra::ConstByteRange Range();

            static bool IsHeaderByte(uint8_t byte);
            static bool IsEscapeByte(uint8_t byte);
            static const size_t HeaderSize = 3;

        private:
            void SetContent(uint16_t content);

        private:
            std::array<uint8_t, HeaderSize> headerContent;
            bool isComplete = false;
            bool isFailed = false;
            uint8_t byteToSet = 0;
        };

        size_t contentToReceive = 0;
        infra::Optional<MessageHeader> messageHeader;

        class State
        {
        public:
            State(ConnectionSerial& connection);
            virtual ~State() = default;

            virtual void EvaluatePendingSend() = 0;
            virtual void AckReceived();
            virtual void Reset() = 0;

            virtual void SizeRequestReceived() = 0;
            virtual void SizeResponseRequestReceived(size_t size) = 0;
            virtual void SizeResponseReceived(size_t size) = 0;
            virtual size_t ContentReceived(infra::ConstByteRange receivedRange) = 0;
            virtual void SizeUpdateReceived(size_t size) = 0;
            virtual void AccumulateWindowUpdateSize(size_t size) = 0;

        protected:
            ConnectionSerial& connection;
            hal::SerialCommunication& serialCommunication;
        };

        class StateInit
            : public State
        {
        public:
            StateInit(ConnectionSerial& connection);

            virtual void EvaluatePendingSend() override;
            virtual void Reset() override;

            virtual void SizeResponseRequestReceived(size_t size) override;
            virtual void SizeResponseReceived(size_t size) override;
            virtual size_t ContentReceived(infra::ConstByteRange receivedRange) override;
            virtual void SizeUpdateReceived(size_t size) override;
            virtual void AccumulateWindowUpdateSize(size_t size) override;

        protected:
            bool sendInProgress = false;
            bool resetAfterSend = false;
            bool responseRequestAfterSend = false;
        };

        class StateInitSizeRequest
            : public StateInit
        {
        public:
            StateInitSizeRequest(ConnectionSerial& connection);

            virtual void Reset() override;
            virtual void SizeRequestReceived() override;
            virtual void SizeResponseRequestReceived(size_t size) override;

        private:
            void SendWindowSizeRequest();

        private:
            bool responseAfterSend = false;
            infra::TimerSingleShot windowRequestTimer;
            MessageHeader windowSizeReqMsg = MessageHeader(MessageType::SizeRequest);
        };

        class StateInitSizeResponseRequest
            : public StateInit
        {
        public:
            StateInitSizeResponseRequest(ConnectionSerial& connection);

            virtual void SizeRequestReceived() override;
            virtual void SizeResponseReceived(size_t size) override;

        private:
            void SendWindowSizeResponseRequest();

            MessageHeader windowSizeRespReqMsg = MessageHeader(MessageType::SizeResponseRequest);
        };

        class StateInitSizeResponse
            : public StateInit
        {
        public:
            StateInitSizeResponse(ConnectionSerial& connection);

            virtual void SizeRequestReceived() override;

        private:
            void SendWindowSizeResponse();

        private:
            MessageHeader windowSizeRespMsg = MessageHeader(MessageType::SizeResponse);
        };

        class StateConnected
            : public State
        {
        public:
            StateConnected(ConnectionSerial& connection);

            virtual void EvaluatePendingSend() override;
            virtual void AckReceived() override;
            virtual void Reset() override;

            virtual void SizeRequestReceived() override;
            virtual void SizeResponseRequestReceived(size_t size) override;
            virtual void SizeResponseReceived(size_t size) override;
            virtual size_t ContentReceived(infra::ConstByteRange receivedRange) override;
            virtual void SizeUpdateReceived(size_t size) override;
            virtual void AccumulateWindowUpdateSize(size_t size) override;

        protected:
            bool IsEscapedByte(uint8_t byte) const;

        private:
            bool UpdatePossible();
            size_t CopyEscapedContent(infra::ConstByteRange receivedRange);
            size_t ContentSizeToSend() const;

        protected:
            infra::ByteRange& sendBuffer;
        };

        class StateConnectedIdle
            : public StateConnected
        {
        public:
            StateConnectedIdle(ConnectionSerial& connection);
        };

        class StateSending
            : public StateConnected
        {
        public:
            StateSending(ConnectionSerial& connection);

            virtual void EvaluatePendingSend() override;
            virtual void Reset() override;

        protected:
            void GoIdle();

        protected:
            bool resetAfterSend = false;
        };

        class StateSendingUpdate
            : public StateSending
        {
        public:
            StateSendingUpdate(ConnectionSerial& connection);

        private:
            void SendUpdate();

        private:
            MessageHeader updateMsg = MessageHeader(MessageType::SizeUpdate);
        };

        class StateConnectedSendingContent
            : public StateSending
        {
        public:
            StateConnectedSendingContent(ConnectionSerial& connection, size_t contentSize);

        private:
            void SendContentWithSize(size_t size);
            void SendPartialContent(infra::ByteRange firstEscapeLessRange, size_t rawContentSize);
            void SendEscapedContent();

        private:
            MessageHeader contentMsgHeader = MessageHeader(MessageType::Content);
            size_t partialContentSizeBeingSent = 0;
            std::array<uint8_t, 2> escapedRange = { { 0xf9, 0 } };
        };

        class StreamWriterWithCyclicBuffer
            : public infra::ByteOutputStreamWriter
        {
        public:
            StreamWriterWithCyclicBuffer(infra::ByteRange& buffer, ConnectionSerial& connection);
            ~StreamWriterWithCyclicBuffer();

        private:
            ConnectionSerial& connection;
        };

        class SerialConnectionStreamReader
            : public infra::StreamReaderWithRewinding
        {
        public:
            explicit SerialConnectionStreamReader(infra::BoundedDeque<uint8_t>& receivedDataQueue);

            virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            virtual bool Empty() const override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual void Rewind(std::size_t marker) override;

            size_t ConsumeExtracted();

        private:
            infra::BoundedDeque<uint8_t>& receivedDataQueue;
            size_t extractedAmount = 0;
        };

    protected:
        size_t pendingSendStream = 0;

    private:
        infra::BoundedDeque<uint8_t>& receivedDataQueue;
        infra::ByteRange sendBuffer;
        infra::ByteRange contentReadyForSend;
        infra::SharedOptional<StreamWriterWithCyclicBuffer> sendStream;
        hal::SerialCommunication& serialCommunication;
        infra::QueueForOneReaderOneIrqWriter<uint8_t> receiveQueue;
        infra::Optional<SerialConnectionStreamReader> receivedDataReader;
        infra::ClaimableResource serialCommunicationResource;
        infra::PolymorphicVariant<State, StateInitSizeRequest, StateInitSizeResponseRequest, StateInitSizeResponse, StateConnectedIdle, StateConnectedSendingContent, StateSendingUpdate> state;
        bool sendStreamAvailable = true;
        size_t minUpdateSize;
        bool observerNotified = false;

        size_t peerBufferSize;
        infra::Optional<size_t> accumulatedWindowSizeUpdate;
        bool updateInProgress = false;
        bool isEscapeSeen = false;
        size_t totalContentSizeToSend = 0;

        infra::Function<void()> onConnected;
        infra::Function<void()> onDisconnected;
    };
}
#endif
