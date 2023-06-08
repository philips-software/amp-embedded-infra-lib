#ifndef NETWORK_CONNECTION_STUB_HPP
#define NETWORK_CONNECTION_STUB_HPP

#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include "gmock/gmock.h"
#include <vector>

namespace services
{
    class ConnectionStub
        : public services::Connection
        , public infra::EnableSharedFromThis<ConnectionStub>
    {
    public:
        void RequestSendStream(std::size_t sendSize) override;
        std::size_t MaxSendStreamSize() const override;
        infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        void AckReceived() override;
        void CloseAndDestroy() override;
        void AbortAndDestroy() override;

        MOCK_METHOD0(CloseAndDestroyMock, void());
        MOCK_METHOD0(AbortAndDestroyMock, void());
        MOCK_CONST_METHOD0(Ipv4Address, IPv4Address());

        void SimulateDataReceived(infra::ConstByteRange data);

        std::vector<uint8_t> sentData;
        std::string SentDataAsString() const;

        void Reset();

    private:
        class StreamWriterStub
            : public infra::StreamWriter
        {
        public:
            explicit StreamWriterStub(ConnectionStub& connection, std::size_t size);

        private:
            void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            std::size_t Available() const override;

        private:
            ConnectionStub& connection;
            std::size_t size;
            std::size_t offset = 0;
        };

        class StreamReaderStub
            : public infra::BoundedDequeInputStreamReader
        {
        public:
            explicit StreamReaderStub(ConnectionStub& connection);

            void ConsumeRead();

        private:
            ConnectionStub& connection;
        };

    private:
        infra::BoundedDeque<uint8_t>::WithMaxSize<4096> receivingData;

        infra::SharedOptional<StreamReaderStub> streamReader;
        infra::SharedOptional<infra::LimitedStreamWriter::WithOutput<infra::StdVectorOutputStreamWriter>> streamWriter;
        infra::SharedPtr<infra::StreamWriter> streamWriterPtr;
    };

    class ConnectionStubWithAckReceivedMock
        : public ConnectionStub
    {
    public:
        void AckReceived() override;
        void CloseAndDestroy() override;
        void AbortAndDestroy() override;

        MOCK_METHOD0(AckReceivedMock, void());
        MOCK_METHOD0(CloseAndDestroyMock, void());
        MOCK_METHOD0(AbortAndDestroyMock, void());
        MOCK_CONST_METHOD0(Ipv4Address, IPv4Address());
    };

    class ConnectionObserverStub
        : public services::ConnectionObserver
    {
    public:
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;

        void SendData(const std::vector<uint8_t>& data);

        std::vector<uint8_t> receivedData;

    private:
        void TryRequestSendStream();

    private:
        std::size_t requestedSendStreamSize = 0;
        std::vector<uint8_t> sendData;
    };
}

#endif
