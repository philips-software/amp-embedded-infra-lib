#include "services/network/test_doubles/ConnectionStub.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"

namespace services
{
    void ConnectionStub::RequestSendStream(std::size_t sendSize)
    {
        assert(streamWriter.Allocatable());
        assert(sendSize <= MaxSendStreamSize());
        streamWriterPtr = streamWriter.Emplace(infra::inPlace, sentData, sendSize);
        infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionStub>& object) {
            infra::SharedPtr<infra::StreamWriter> stream = std::move(object->streamWriterPtr);
            object->Observer().SendStreamAvailable(std::move(stream));
        },
            SharedFromThis());
    }

    std::size_t ConnectionStub::MaxSendStreamSize() const
    {
        return 1024;
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionStub::ReceiveStream()
    {
        assert(streamReader.Allocatable());
        return streamReader.Emplace(*this);
    }

    void ConnectionStub::AckReceived()
    {
        if (streamReader)
            streamReader->ConsumeRead();
    }

    void ConnectionStub::CloseAndDestroy()
    {
        CloseAndDestroyMock();
        Detach();
    }

    void ConnectionStub::AbortAndDestroy()
    {
        AbortAndDestroyMock();
        Detach();
    }

    void ConnectionStub::SimulateDataReceived(infra::ConstByteRange data)
    {
        receivingData.insert(receivingData.end(), data.begin(), data.end());
        if (IsAttached())
            Observer().DataReceived();
    }

    std::string ConnectionStub::SentDataAsString() const
    {
        return { sentData.begin(), sentData.end() };
    }

    ConnectionStub::StreamWriterStub::StreamWriterStub(ConnectionStub& connection, std::size_t size)
        : connection(connection)
        , size(size)
    {}

    void ConnectionStub::StreamWriterStub::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        assert(range.size() + offset <= size);
        offset += range.size();
        connection.sentData.insert(connection.sentData.end(), range.begin(), range.end());
    }

    std::size_t ConnectionStub::StreamWriterStub::Available() const
    {
        return size - offset;
    }

    ConnectionStub::StreamReaderStub::StreamReaderStub(ConnectionStub& connection)
        : infra::BoundedDequeInputStreamReader(connection.receivingData)
        , connection(connection)
    {}

    void ConnectionStub::StreamReaderStub::ConsumeRead()
    {
        connection.receivingData.erase(connection.receivingData.begin(), connection.receivingData.begin() + ConstructSaveMarker());
        Rewind(0);
    }

    void ConnectionStubWithAckReceivedMock::AckReceived()
    {
        AckReceivedMock();
        ConnectionStub::AckReceived();
    }

    void ConnectionStubWithAckReceivedMock::CloseAndDestroy()
    {
        CloseAndDestroyMock();
        Detach();
    }

    void ConnectionStubWithAckReceivedMock::AbortAndDestroy()
    {
        AbortAndDestroyMock();
        Detach();
    }

    void ConnectionObserverStub::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        infra::ConstByteRange data(sendData.data(), sendData.data() + requestedSendStreamSize);
        stream << data;
        writer = nullptr;
        sendData.erase(sendData.begin(), sendData.begin() + requestedSendStreamSize);
        requestedSendStreamSize = 0;
        TryRequestSendStream();
    }

    void ConnectionObserverStub::DataReceived()
    {
        infra::SharedPtr<infra::StreamReader> reader = Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        for (infra::ConstByteRange received = stream.ContiguousRange(); !received.empty(); received = stream.ContiguousRange())
            receivedData.insert(receivedData.end(), received.begin(), received.end());

        Subject().AckReceived();
    }

    void ConnectionObserverStub::SendData(const std::vector<uint8_t>& data)
    {
        sendData.insert(sendData.end(), data.begin(), data.end());

        TryRequestSendStream();
    }

    void ConnectionObserverStub::TryRequestSendStream()
    {
        if (requestedSendStreamSize == 0 && !sendData.empty())
        {
            requestedSendStreamSize = std::min(sendData.size(), Subject().MaxSendStreamSize());
            Subject().RequestSendStream(requestedSendStreamSize);
        }
    }
}
