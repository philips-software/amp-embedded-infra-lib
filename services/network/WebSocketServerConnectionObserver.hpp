#ifndef SERVICES_WEB_SOCKET_SERVER_CONNECTION_OBSERVER_HPP
#define SERVICES_WEB_SOCKET_SERVER_CONNECTION_OBSERVER_HPP

#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include "services/network/WebSocket.hpp"

namespace services
{
    class WebSocketServerConnectionObserver
        : public services::ConnectionObserver
        , public services::Connection
    {
    public:
        template<std::size_t SendBufferSize, std::size_t ReceiveBufferSize>
        using WithBufferSizes = infra::WithStorage<infra::WithStorage<WebSocketServerConnectionObserver, infra::BoundedVector<uint8_t>::WithMaxSize<SendBufferSize>>,
            infra::BoundedDeque<uint8_t>::WithMaxSize<ReceiveBufferSize>>;

        WebSocketServerConnectionObserver(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedDeque<uint8_t>& receiveBuffer);

    public:
        // Implementation of ConnectionObserver
        void Attached() override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Detaching() override;

        // Implementation of Connection
        void RequestSendStream(std::size_t sendSize) override;
        std::size_t MaxSendStreamSize() const override;
        infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        void AckReceived() override;
        void CloseAndDestroy() override;
        void AbortAndDestroy() override;

    private:
        void ReceiveStreamAllocatable();
        void SendStreamAllocatable();
        void SetReceivingStateReceiveHeader();
        void SetStateReceiveData(services::WebSocketFrameHeader header);
        void SetReceivingStateClose();
        void SetReceivingStatePong();
        void SetStateSendingIdle();
        void SendFrame(services::WebSocketOpCode operationCode, infra::ConstByteRange data, infra::StreamWriter& writer) const;
        void TryAllocateSendStream();

    private:
        class ReceivingState
        {
        public:
            ReceivingState() = default;
            ReceivingState(const ReceivingState& other) = delete;
            ReceivingState& operator=(const ReceivingState& other) = delete;
            virtual ~ReceivingState() = default;

        public:
            virtual void DataReceived();
        };

        class ReceivingStateThatSendsData
            : public ReceivingState
        {
        public:
            explicit ReceivingStateThatSendsData(WebSocketServerConnectionObserver& connection);

            virtual void Send(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
        };

        class ReceivingStateReceiveHeader
            : public ReceivingState
        {
        public:
            explicit ReceivingStateReceiveHeader(WebSocketServerConnectionObserver& connection);

            void DataReceived() override;

        private:
            WebSocketServerConnectionObserver& connection;
        };

        class ReceivingStateReceiveData
            : public ReceivingState
        {
        public:
            ReceivingStateReceiveData(WebSocketServerConnectionObserver& connection, const services::WebSocketFrameHeader& header);

            void DataReceived() override;

        private:
            void ReceiveData();
            void ConsumeData(infra::DataInputStream& stream);
            void DiscardRest(infra::DataInputStream& stream);
            void ProcessFrameData(uint32_t alreadyReceived);
            void ProcessPongData(uint32_t alreadyReceived);
            void SetNextState();

        private:
            WebSocketServerConnectionObserver& connection;
            services::WebSocketFrameHeader header;
            uint32_t sizeToReceive;
            uint32_t receiveOffset = 0;
        };

        class ReceivingStateClose
            : public ReceivingStateThatSendsData
        {
        public:
            explicit ReceivingStateClose(WebSocketServerConnectionObserver& connection);

            void Send(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            WebSocketServerConnectionObserver& connection;
        };

        class ReceivingStatePong
            : public ReceivingStateThatSendsData
        {
        public:
            explicit ReceivingStatePong(WebSocketServerConnectionObserver& connection);

            void Send(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            WebSocketServerConnectionObserver& connection;
        };

        class SendingState
        {
        public:
            SendingState() = default;
            SendingState(const SendingState& other) = delete;
            SendingState& operator=(const SendingState& other) = delete;
            virtual ~SendingState() = default;

        public:
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;
            virtual void CheckForSomethingToDo();
        };

        class SendingStateIdle
            : public SendingState
        {
        public:
            explicit SendingStateIdle(WebSocketServerConnectionObserver& connection);

            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
            void CheckForSomethingToDo() override;

        private:
            WebSocketServerConnectionObserver& connection;
        };

        class SendingStateInternalData
            : public SendingState
        {
        public:
            explicit SendingStateInternalData(WebSocketServerConnectionObserver& connection);

            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            WebSocketServerConnectionObserver& connection;
        };

        class SendingStateExternalData
            : public SendingState
        {
        public:
            explicit SendingStateExternalData(WebSocketServerConnectionObserver& connection);

            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            WebSocketServerConnectionObserver& connection;
        };

    private:
        ReceivingStateThatSendsData* receivingStateThatWantsToSendData = nullptr;
        infra::PolymorphicVariant<ReceivingState, ReceivingStateReceiveHeader, ReceivingStateReceiveData,
            ReceivingStateClose, ReceivingStatePong>
            receivingState;
        infra::PolymorphicVariant<SendingState, SendingStateIdle, SendingStateInternalData, SendingStateExternalData> sendingState;
        infra::BoundedDeque<uint8_t>& receiveBuffer;
        infra::BoundedVector<uint8_t>& sendBuffer;
        std::size_t requestedSendSize = 0;
        infra::NotifyingSharedOptional<infra::LimitedStreamReaderWithRewinding::WithInput<infra::BoundedDequeInputStreamReader>> streamReader;
        infra::SharedPtr<void> keepAliveWhileReading;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> streamWriter;
        infra::SharedPtr<void> keepAliveWhileWriting;
        bool sendBufferReadyForSending = false;
        infra::BoundedVector<uint8_t>::WithMaxSize<8> pongBuffer;
    };
}

#endif
