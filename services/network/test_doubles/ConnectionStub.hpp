#ifndef NETWORK_CONNECTION_STUB_HPP
#define NETWORK_CONNECTION_STUB_HPP

#include "gmock/gmock.h"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include <vector>

namespace services
{
    //TICS -INT#002: A mock or stub may have public data
    //TICS -INT#027: MOCK_METHOD can't add 'virtual' to its signature
    class ConnectionStub
        : public services::Connection
        , public infra::EnableSharedFromThis<ConnectionStub>
    {
    public:
        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

        MOCK_METHOD0(CloseAndDestroyMock, void());
        MOCK_METHOD0(AbortAndDestroyMock, void());
        MOCK_CONST_METHOD0(Ipv4Address, IPv4Address());

        void SimulateDataReceived(infra::ConstByteRange data);

        std::vector<uint8_t> sentData;
        std::string SentDataAsString() const;

    private:
        class StreamWriterStub
            : public infra::StreamWriter
        {
        public:
            explicit StreamWriterStub(ConnectionStub& connection);

        private:
            virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual std::size_t Available() const override;

        private:
            ConnectionStub& connection;
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
        infra::SharedOptional<StreamWriterStub> streamWriter;
        infra::SharedPtr<infra::StreamWriter> streamWriterPtr;
    };

    class ConnectionStubWithAckReceivedMock
        : public ConnectionStub
    {
    public:
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

        MOCK_METHOD0(AckReceivedMock, void());
        MOCK_METHOD0(CloseAndDestroyMock, void());
        MOCK_METHOD0(AbortAndDestroyMock, void());
        MOCK_CONST_METHOD0(Ipv4Address, IPv4Address());
    };

    class ConnectionObserverStub
        : public services::ConnectionObserver
    {
    public:
        ConnectionObserverStub() = default;
        ConnectionObserverStub(services::Connection& connection)
            : services::ConnectionObserver(connection)
        {}

        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

        void SendData(const std::vector<uint8_t>& data);

        std::vector<uint8_t> receivedData;

        using services::ConnectionObserver::Subject;

    private:
        void TryRequestSendStream();

    private:
        std::size_t requestedSendStreamSize = 0;
        std::vector<uint8_t> sendData;
    };
}

#endif
