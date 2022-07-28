#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/network/ConnectionSerial.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"

class ConnectionSerialTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    ConnectionSerialTest()
        : firstRequest1([this]() { ExpectWindowSizeRequest([this]() { serialCommunication.actionOnCompletion(); }); })
        , receiveBuffer(2048)
        , receivedDataQueue(infra::inPlace, storage)
        , connection(infra::inPlace, infra::MakeByteRange(sendBuffer), infra::MemoryRange<uint8_t>(receiveBuffer), *receivedDataQueue,  serialCommunication, infra::emptyFunction, infra::emptyFunction)
        , firstRequest2([this]() { ExecuteAllActions(); })
    {
        connection->Attach(infra::UnOwnedSharedPtr(observer));
    }

    testing::StrictMock<hal::SerialCommunicationMockWithAssert> serialCommunication;
    infra::Execute firstRequest1;
    uint8_t sendBuffer[4096];
    std::vector<uint8_t> receiveBuffer;
    infra::Optional<infra::BoundedDeque<uint8_t>> receivedDataQueue;
    std::array<infra::StaticStorage<uint8_t>, 2048> storage;
    infra::Optional<services::ConnectionSerial> connection;
    testing::StrictMock<services::ConnectionObserverMock> observer;
    infra::Execute firstRequest2;

public:
    void WindowSizeRequestReceived()
    {
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
    }

    void WindowSizeResponseReceived(size_t windowSize = 2048)
    {
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfb });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(windowSize >> 8) });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(windowSize) });
    }

    void WindowSizeResponseRequestReceived(size_t windowSize = 2048)
    {
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfc });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(windowSize >> 8) });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(windowSize) });
    }

    void ContentMessageReceived(std::vector<uint8_t> msg)
    {
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfd });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(msg.size() >> 8) });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(msg.size()) });

        for (uint8_t element : msg)
            serialCommunication.dataReceived(std::vector<uint8_t>{ element });
    }

    void ContentMessageReceivedAndObserverNotified(std::vector<uint8_t> msg)
    {
        ContentMessageReceived(msg);
        EXPECT_CALL(observer, DataReceived());
        ExecuteAllActions();
    }

    void WindowSizeUpdateReceived(size_t windowSize)
    {
        serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfe });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(windowSize >> 8) });
        serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(windowSize) });
    }

    void ExpectWindowSizeRequest(infra::Function<void()> actionOnCompletion)
    {
        EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>{ 0xfa, 0xfa, 0xfa }))
            .WillOnce(testing::InvokeWithoutArgs([actionOnCompletion]() { actionOnCompletion(); }));
    }

    void ExpectWindowSizeRequest()
    {
        EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>{ 0xfa, 0xfa, 0xfa }));
    }

    void ExpectWindowSizeResponse(size_t windowSize)
    {
        EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>{ 0xfb, static_cast<uint8_t>(windowSize >> 8), static_cast<uint8_t>(windowSize) }));
    }

    void ExpectWindowSizeResponseRequest(size_t windowSize)
    {
        EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>{ 0xfc, static_cast<uint8_t>(windowSize >> 8), static_cast<uint8_t>(windowSize) }));
    }

    void ExpectWindowSizeUpdate(size_t windowSize)
    {
        EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>{ 0xfe, static_cast<uint8_t>(windowSize >> 8), static_cast<uint8_t>(windowSize) }));
    }

    void ExpectWindowSizeUpdateWithComplete(size_t windowSize)
    {
        EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>{ 0xfe, static_cast<uint8_t>(windowSize >> 8), static_cast<uint8_t>(windowSize) }))
            .WillOnce(testing::InvokeWithoutArgs([this]() { serialCommunication.actionOnCompletion(); }));
    }

    void ExpectAndCompleteWindowSizeRequest()
    {
        ExpectWindowSizeRequest();
        ExecuteAllActions();
        serialCommunication.actionOnCompletion();
    }

    void ExpectAndCompleteWindowSizeResponse(size_t windowSize)
    {
        ExpectWindowSizeResponse(windowSize);
        ExecuteAllActions();
        serialCommunication.actionOnCompletion();
    }

    void ExpectAndCompleteWindowSizeResponseRequest(size_t windowSize)
    {
        ExpectWindowSizeResponseRequest(windowSize);

        ExecuteAllActions();
        serialCommunication.actionOnCompletion();
    }

    uint8_t ExpectSepearateContentHeaderAndContentMessages(std::vector<uint8_t> msg)
    {
        testing::InSequence sequential;

        int escapedMsgSize = 0;
        for (std::size_t i = 0; i < msg.size(); i++)
        {
            if (msg[i] >= 0xf9 && msg[i] <= 0xfe)
                ++escapedMsgSize;
            ++escapedMsgSize;
        }

        uint8_t serialCallCount = 0;
        std::vector<uint8_t> contentMsgHeader = { 0xfd, static_cast<uint8_t>(escapedMsgSize >> 8), static_cast<uint8_t>(escapedMsgSize) };
        EXPECT_CALL(serialCommunication, SendDataMock(contentMsgHeader));
        ++serialCallCount;

        std::vector<uint8_t>::iterator it1 = msg.begin(), it2 = msg.begin();

        for (it2 = msg.begin(); it2 != msg.end(); ++it2)
        {
            if (*it2 >= 0xf9 && *it2 <= 0xfe)
            {
                if (it1 != it2)
                {
                    EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>(it1, it2)));
                    ++serialCallCount;
                }

                EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>({ 0xf9, *it2 })));
                ++serialCallCount;
                it1 = it2 + 1;
            }
        }

        if (it1 != it2)
        {
            EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>(it1, it2)));
            ++serialCallCount;
        }

        return serialCallCount;
    }

    void ExpectAndCompleteSepearateContentHeaderAndContentMessage(std::vector<uint8_t> msg)
    {
        uint8_t serialCallCount = ExpectSepearateContentHeaderAndContentMessages(msg);
        ExecuteAllActions();
        for (int i=0; i<serialCallCount; i++)
            serialCommunication.actionOnCompletion();
    }

    void ExpectSendStreamAvailable(std::vector<uint8_t> msg)
    {
        EXPECT_CALL(observer, SendStreamAvailable(testing::_)).WillOnce(testing::Invoke([msg](infra::SharedPtr<infra::StreamWriter> writer)
        {
            infra::ByteOutputStream::WithErrorPolicy stream(*writer);
            stream << infra::ConstByteRange(msg);
        }));
    }

    void GetAndWriteToSendStream(std::vector<uint8_t> msg)
    {
        connection->RequestSendStream(connection->MaxSendStreamSize());
        ExpectSendStreamAvailable(msg);
    }

    void ExpectAndCompleteUpdate(size_t windowSize)
    {
        ExpectWindowSizeUpdateWithComplete(windowSize);
        ExecuteAllActions();
    }

    void AckReceivedAndExpectUpdate(size_t windowSize)
    {
        ExpectWindowSizeUpdateWithComplete(windowSize);
        connection->AckReceived();
        ExecuteAllActions();
    }

    void Initialization(size_t peerWindowSize = 2048)
    {
        WindowSizeResponseRequestReceived(peerWindowSize);
        ExpectAndCompleteWindowSizeResponse(ReceiveBufferSize());
        ExecuteAllActions();
    }

    void ReconstructConnection()
    {
        observer.Detach();
        connection.Emplace(infra::MakeByteRange(sendBuffer), infra::MemoryRange<uint8_t>(receiveBuffer), *receivedDataQueue, serialCommunication, infra::emptyFunction, infra::emptyFunction);
        connection->Attach(infra::UnOwnedSharedPtr(observer));
    }

    void ReconstructConnectionWithMinUpdateSize(size_t minUpdateSize)
    {
        ExpectWindowSizeRequest([this]() { serialCommunication.actionOnCompletion(); });
        observer.Detach();
        connection.Emplace(infra::MakeByteRange(sendBuffer), infra::MemoryRange<uint8_t>(receiveBuffer), *receivedDataQueue, serialCommunication, infra::emptyFunction, infra::emptyFunction, minUpdateSize);
        connection->Attach(infra::UnOwnedSharedPtr(observer));
        ExecuteAllActions();
    }

    void ReconstructConnectionWithReceiveBufferSize(size_t size)
    {
        ExpectWindowSizeRequest([this]() { serialCommunication.actionOnCompletion(); });
        receiveBuffer.resize(size);
        receivedDataQueue.Emplace(infra::MemoryRange<infra::StaticStorage<uint8_t>>(&storage[0], &storage[size]));
        ReconstructConnection();
        ExecuteAllActions();
    }

    size_t ReceiveBufferSize() const
    {
        return static_cast<uint16_t>(receiveBuffer.size() - 1);
    }
};

TEST_F(ConnectionSerialTest, MaxSendStreamSize)
{
    EXPECT_EQ(sizeof(sendBuffer), connection->MaxSendStreamSize());
}


//Window size request
TEST_F(ConnectionSerialTest, sends_request_for_window_size_after_construction_at_every_second)
{
    ExpectWindowSizeRequest();
    ExecuteAllActions();

    ForwardTime(std::chrono::seconds(1));
    serialCommunication.actionOnCompletion();

    ExpectWindowSizeRequest();
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(ConnectionSerialTest, sends_request_for_window_size_after_construction_at_every_second2)
{
    WindowSizeResponseReceived();
    ExecuteAllActions();

    ExpectWindowSizeRequest();
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(ConnectionSerialTest, stops_sending_window_size_request_after_receiving_window_size_response_request_and_sends_window_size_response)
{
    WindowSizeResponseRequestReceived();
    ExpectAndCompleteWindowSizeResponse(ReceiveBufferSize());
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(ConnectionSerialTest, stops_sending_window_size_request_after_receiving_window_size_request_and_sends_window_size_response_request)
{
    ExpectWindowSizeRequest();
    ReconstructConnection();

    WindowSizeRequestReceived();
    ExecuteAllActions();

    ExpectWindowSizeResponseRequest(ReceiveBufferSize());
    serialCommunication.actionOnCompletion();
}

TEST_F(ConnectionSerialTest, stops_sending_window_size_request_after_receiving_window_size_request_and_sends_window_size_response_request2)
{
    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(ConnectionSerialTest, window_response_request_received_while_sending_size_request)
{
    ExpectWindowSizeRequest();
    ForwardTime(std::chrono::seconds(1));

    WindowSizeResponseRequestReceived();
    ExecuteAllActions();

    ExpectWindowSizeResponse(ReceiveBufferSize());
    serialCommunication.actionOnCompletion();
}

TEST_F(ConnectionSerialTest, discards_scrap_bytes)
{
    serialCommunication.dataReceived(std::vector<uint8_t>{ 'a' });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 'b' });

    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());
}

TEST_F(ConnectionSerialTest, handle_partially_received_header)
{
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfc });
    serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(2048 >> 8) });
    ExecuteAllActions();
    serialCommunication.dataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(2048 & 0xff) });
    ExpectAndCompleteWindowSizeResponse(ReceiveBufferSize());
}

//Window size response request
TEST_F(ConnectionSerialTest, size_request_received_while_sending_response_request_triggers_another_response_request_after_send)
{
    WindowSizeRequestReceived();
    ExpectWindowSizeResponseRequest(ReceiveBufferSize());
    ExecuteAllActions();

    WindowSizeRequestReceived();
    ExecuteAllActions();

    ExpectWindowSizeResponseRequest(ReceiveBufferSize());
    serialCommunication.actionOnCompletion();
}

TEST_F(ConnectionSerialTest, handle_wrapped_header)
{
    ReconstructConnectionWithReceiveBufferSize(8);

    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());

    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());

    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());
}

//Window size response
TEST_F(ConnectionSerialTest, window_size_response_intercepted_by_window_size_request_resets_connection)
{
    WindowSizeResponseRequestReceived();
    ExpectWindowSizeResponse(ReceiveBufferSize());
    WindowSizeRequestReceived();
    ExecuteAllActions();

    ExpectWindowSizeResponseRequest(ReceiveBufferSize());
    serialCommunication.actionOnCompletion();
}

TEST_F(ConnectionSerialTest, RequestSendStream_before_initialization_grants_SendStreamAvailable_but_does_not_start_sending_if_initialization_is_not_complete)
{
    GetAndWriteToSendStream({ '1', '2' });
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, RequestSendStream_twice_before_initialization_grants_SendStreamAvailable_only_once)
{
    GetAndWriteToSendStream({ '1', '2' });
    ExecuteAllActions();

    connection->RequestSendStream(connection->MaxSendStreamSize());
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, RequestSendStream_before_initialization_grants_SendStreamAvailable_but_does_not_start_sending_if_initialization_is_not_complete2)
{
    GetAndWriteToSendStream({ '1', '2' });
    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());
}

TEST_F(ConnectionSerialTest, RequestSendStream_before_initialization_grants_SendStreamAvailable_and_starts_sending_after_initialization)
{
    GetAndWriteToSendStream({ '1', '2' });
    WindowSizeResponseRequestReceived();
    ExpectWindowSizeResponse(ReceiveBufferSize());
    ExecuteAllActions();
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '1', '2' });
    serialCommunication.actionOnCompletion();
}

TEST_F(ConnectionSerialTest, RequestSendStream_before_initialization_grants_SendStreamAvailable_and_starts_sending_after_initialization2)
{
    GetAndWriteToSendStream({ '1', '2' });
    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeResponseRequest(ReceiveBufferSize());
    WindowSizeResponseReceived();
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '1', '2' });
}

TEST_F(ConnectionSerialTest, RequestSendStream_after_initialization_grants_SendStreamAvailable_and_starts_sending_content)
{
    Initialization();

    GetAndWriteToSendStream({ '1', '2' });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '1', '2' });
}

TEST_F(ConnectionSerialTest, unexpected_request_after_initialization_blocks_send_and_resets)
{
    Initialization();

    GetAndWriteToSendStream({ '1', '2' });
    WindowSizeRequestReceived();
    ExpectAndCompleteWindowSizeRequest();
    Initialization();
    GetAndWriteToSendStream({ '2', '3' });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '2', '3' });
}

TEST_F(ConnectionSerialTest, unexpected_response_request_after_initialization_blocks_send_and_resets)
{
    Initialization();

    WindowSizeResponseRequestReceived();
    ExpectAndCompleteWindowSizeRequest();
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, unexpected_response_after_initialization_blocks_send_and_resets)
{
    Initialization();

    WindowSizeResponseReceived();
    ExpectAndCompleteWindowSizeRequest();
    ExecuteAllActions();
}


TEST_F(ConnectionSerialTest, escape_header_and_escape_bytes_in_content)
{
    Initialization();

    GetAndWriteToSendStream({ 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe });

    GetAndWriteToSendStream({ 0xf9, '1' });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ 0xf9, '1' });

    GetAndWriteToSendStream({ '1', 0xf9 });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '1', 0xf9 });

    GetAndWriteToSendStream({ 0xf9, '1', '2', '3', 0xf9 });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ 0xf9, '1', '2', '3', 0xf9 });
}

TEST_F(ConnectionSerialTest, only_send_content_with_size_less_than_available_peer_space_minus_one_update_message_size)
{
    ReconstructConnectionWithMinUpdateSize(20);
    Initialization(10);

    // 'max content message size(4)' = 'peer space(10)' - 'update message size(3)' - 'content message header size(3)'
    GetAndWriteToSendStream({ '1', '2', '3', '4', '5', '6' });
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '1', '2', '3', '4' });

    WindowSizeUpdateReceived(4);
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '5' });

    WindowSizeUpdateReceived(3);
    ExecuteAllActions();

    WindowSizeUpdateReceived(1);
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '6' });
}

TEST_F(ConnectionSerialTest, do_not_schedule_SendStreamAvailable_until_content_has_been_sent)
{
    Initialization(8);

    GetAndWriteToSendStream({ '1', '2', '3', '4' });
    ExpectSepearateContentHeaderAndContentMessages({ '1', '2' });
    ExecuteAllActions();

    connection->RequestSendStream(connection->MaxSendStreamSize());
    ExecuteAllActions();

    serialCommunication.actionOnCompletion();
    serialCommunication.actionOnCompletion();
    ExecuteAllActions();

    WindowSizeUpdateReceived(5);
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '3', '4' });

    ExpectSendStreamAvailable(std::vector<uint8_t>({ '5', '6' }));
    ExecuteAllActions();

    WindowSizeUpdateReceived(8);
    ExpectWindowSizeUpdateWithComplete(6);
    ExpectAndCompleteSepearateContentHeaderAndContentMessage({ '5', '6' });
}

TEST_F(ConnectionSerialTest, notify_observer_when_content_message_received_only_if_observer_is_unaware)
{
    ReconstructConnectionWithMinUpdateSize(2000);
    Initialization();

    ContentMessageReceived({ '1', '2', '3' });
    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();

    ContentMessageReceived({ '4', '5', '6' });
    ExecuteAllActions();

    connection->ReceiveStream();
    ContentMessageReceived({ '7', '8', '9' });
    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, notify_observer_when_partial_content_message_received)
{
    ReconstructConnectionWithMinUpdateSize(2000);
    Initialization();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfd });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 2 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ '1' });

    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, notify_observer_when_content_message_received_part_by_part)
{
    Initialization();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfd });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 2 });

    serialCommunication.dataReceived(std::vector<uint8_t>{ '1' });
    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();

    connection->ReceiveStream();

    serialCommunication.dataReceived(std::vector<uint8_t>{ '2' });
    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, extract_received_content_via_StreamReader_using_ExtractContiguousRange)
{
    ReconstructConnectionWithMinUpdateSize(2000);
    Initialization();
    ContentMessageReceived({ '1', });
    ContentMessageReceivedAndObserverNotified({ '2', '3' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();

    EXPECT_FALSE(reader->Empty());
    EXPECT_EQ(3, reader->Available());

    EXPECT_EQ(std::vector<uint8_t>({ '1', '2' }), reader->ExtractContiguousRange(2));
    EXPECT_EQ(1, reader->Available());

    EXPECT_EQ(std::vector<uint8_t>({ '3' }), reader->ExtractContiguousRange(2));
    EXPECT_EQ(0, reader->Available());

    EXPECT_EQ(std::vector<uint8_t>({}), reader->ExtractContiguousRange(1));
    EXPECT_TRUE(reader->Empty());
}

TEST_F(ConnectionSerialTest, extract_received_content_via_StreamReader_using_Extract)
{
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    infra::Optional<infra::StreamErrorPolicy> error;
    std::vector<uint8_t> a;

    error.Emplace(infra::softFail);
    a.resize(5, 0);
    reader->Extract(infra::ByteRange(a), *error);
    EXPECT_TRUE(error->Failed());
    EXPECT_EQ(4, reader->Available());

    error.Emplace(infra::softFail);
    a.resize(1, 0);
    reader->Extract(infra::ByteRange(a), *error);
    EXPECT_EQ(std::vector<uint8_t>({ '1' }), a);
    EXPECT_FALSE(error->Failed());
    EXPECT_EQ(3, reader->Available());

    error.Emplace(infra::softFail);
    a.resize(3, 0);
    reader->Extract(infra::ByteRange(a), *error);
    EXPECT_EQ(std::vector<uint8_t>({ '2', '3', '4' }), a);
    EXPECT_FALSE(error->Failed());
    EXPECT_EQ(0, reader->Available());
}

TEST_F(ConnectionSerialTest, peek_received_content_via_StreamReader_using_PeekContiguousRange)
{
    ReconstructConnectionWithMinUpdateSize(2000);
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();

    EXPECT_EQ(std::vector<uint8_t>({ '1', '2', '3', '4' }), reader->PeekContiguousRange(0));
    EXPECT_EQ(std::vector<uint8_t>({ '2', '3', '4' }), reader->PeekContiguousRange(1));
    EXPECT_EQ(std::vector<uint8_t>({}), reader->PeekContiguousRange(4));
    EXPECT_EQ(4, reader->Available());

    reader->ExtractContiguousRange(1);
    EXPECT_EQ(std::vector<uint8_t>({ '2', '3', '4' }), reader->PeekContiguousRange(0));
    EXPECT_EQ(3, reader->Available());
}

TEST_F(ConnectionSerialTest, peek_received_content_via_StreamReader_using_Peek)
{
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    infra::StreamErrorPolicy error(infra::softFail);

    EXPECT_EQ('1', reader->Peek(error));
    EXPECT_FALSE(error.Failed());

    reader->ExtractContiguousRange(1);

    EXPECT_EQ('2', reader->Peek(error));
    EXPECT_FALSE(error.Failed());

    reader->ExtractContiguousRange(2);

    reader->Peek(error);
    EXPECT_TRUE(error.Failed());
}

TEST_F(ConnectionSerialTest, AckReceived_consumes_content_extracted_via_StreamReader)
{
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4', '5' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    EXPECT_EQ(std::vector<uint8_t>({ '1' }), reader->ExtractContiguousRange(1));
    AckReceivedAndExpectUpdate(4);

    reader = connection->ReceiveStream();
    EXPECT_EQ(4, reader->Available());
    EXPECT_EQ(std::vector<uint8_t>({ '2', '3', '4', '5' }), reader->ExtractContiguousRange(4));
    AckReceivedAndExpectUpdate(4);
}

TEST_F(ConnectionSerialTest, accumulate_size_updates_until_current_one_is_completed)
{
    Initialization();

    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4', '5', '6', '7', '8', '9' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    ExpectWindowSizeUpdate(4);
    connection->AckReceived();
    ExecuteAllActions();

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(4);
    connection->AckReceived();

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(4);
    connection->AckReceived();

    ExpectWindowSizeUpdate(8);
    serialCommunication.actionOnCompletion();
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, size_updates_consumes_peer_buffer_space)
{
    Initialization(7);
    ContentMessageReceivedAndObserverNotified({ '1' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    AckReceivedAndExpectUpdate(4);

    GetAndWriteToSendStream({ '1' });
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, dont_send_size_updates_less_than_default_minimum_size_of_4)
{
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4', '5' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    AckReceivedAndExpectUpdate(4);

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(3);
    connection->AckReceived();
    ExecuteAllActions();

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    ExpectWindowSizeUpdate(4);
    connection->AckReceived();
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, dont_send_size_updates_less_than_configured_size)
{
    ReconstructConnectionWithMinUpdateSize(5);
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    connection->AckReceived();
    ExecuteAllActions();

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    ExpectWindowSizeUpdate(5);
    connection->AckReceived();
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, dont_send_size_updates_when_peer_buffer_is_completely_full)
{
    Initialization(6);
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4', '5', '6', '7', '8', '9' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(1);
    AckReceivedAndExpectUpdate(4);

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(4);
    AckReceivedAndExpectUpdate(4);

    reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(4);
    connection->AckReceived();
    ExecuteAllActions();

    WindowSizeUpdateReceived(3);
    ExpectWindowSizeUpdate(7);
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, send_size_updates_for_consumed_headers)
{
    Initialization();

    ContentMessageReceived({ '1' });
    ContentMessageReceived({ '2' });
    ExpectWindowSizeUpdate(6);
    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, send_size_updates_for_consumed_headers2)
{
    Initialization();

    ExpectWindowSizeUpdate(6);
    WindowSizeUpdateReceived(0);
    WindowSizeUpdateReceived(0);
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, escape_bytes_immediately_included_in_size_update)
{
    Initialization();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfd });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 9 });

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    EXPECT_CALL(observer, DataReceived());
    ExpectAndCompleteUpdate(4);
    ExecuteAllActions();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfa });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });

    ExpectAndCompleteUpdate(4);
}

TEST_F(ConnectionSerialTest, call_to_ReceiveStream_returns_stream_with_updated_messages)
{
    ReconstructConnectionWithMinUpdateSize(2000);
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    ContentMessageReceivedAndObserverNotified({ '2' });
    EXPECT_EQ(2, reader->Available());
    EXPECT_EQ(std::vector<uint8_t>({ '1', '2' }), reader->ExtractContiguousRange(2));
}

TEST_F(ConnectionSerialTest, handle_wrapping_while_extracting_content_with_ExtractContiguousRange)
{
    receiveBuffer.resize(10);
    receivedDataQueue.Emplace(infra::MemoryRange<infra::StaticStorage<uint8_t>>(&storage[0], &storage[10]));

    ExpectWindowSizeRequest([this]() { serialCommunication.actionOnCompletion(); });
    ReconstructConnection();

    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4', '5' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    EXPECT_EQ(std::vector<uint8_t>({ '1', '2', '3', '4', '5' }), reader->ExtractContiguousRange(5));
    AckReceivedAndExpectUpdate(8);

    ContentMessageReceivedAndObserverNotified({ '7', '8', '9', 'a', 'b', 'c' });

    reader = connection->ReceiveStream();
    EXPECT_EQ(std::vector<uint8_t>({ '7', '8', '9', 'a', 'b' }), reader->PeekContiguousRange(0));
    EXPECT_EQ(std::vector<uint8_t>({ '7', '8', '9', 'a', 'b' }), reader->ExtractContiguousRange(6));
    AckReceivedAndExpectUpdate(8);

    reader = connection->ReceiveStream();
    EXPECT_EQ(std::vector<uint8_t>({ 'c' }), reader->ExtractContiguousRange(2));
    connection->AckReceived();
    ExecuteAllActions();
}

TEST_F(ConnectionSerialTest, handle_wrapping_while_extracting_content_with_Extract)
{
    ReconstructConnectionWithReceiveBufferSize(10);
    Initialization();
    ContentMessageReceivedAndObserverNotified({ '1', '2', '3', '4', '5' });

    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    reader->ExtractContiguousRange(5);
    AckReceivedAndExpectUpdate(8);

    ContentMessageReceivedAndObserverNotified({ '7', '8', '9', 'a', 'b', 'c' });

    std::vector<uint8_t> a;
    infra::Optional<infra::StreamErrorPolicy> error;
    error.Emplace(infra::softFail);
    a.resize(6, 0);

    reader = connection->ReceiveStream();
    reader->Extract(infra::ByteRange(a), *error);

    EXPECT_EQ(std::vector<uint8_t>({ '7', '8', '9', 'a', 'b', 'c' }), a);
    EXPECT_FALSE(error->Failed());
    EXPECT_EQ(0, reader->Available());
}

TEST_F(ConnectionSerialTest, handle_escaped_bytes_while_extracting_content)
{
    Initialization();

    ExpectWindowSizeUpdateWithComplete(9);
    ContentMessageReceivedAndObserverNotified({ 0xf9, 0xf9, 0xf9, 0xfa, 0xf9, 0xfb, 0xf9, 0xfc, 0xf9, 0xfd, 0xf9, 0xfe });
    infra::SharedPtr<infra::StreamReader> reader = connection->ReceiveStream();
    EXPECT_EQ(6, reader->Available());
    EXPECT_EQ(std::vector<uint8_t>({ 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe }), reader->ExtractContiguousRange(6));
    AckReceivedAndExpectUpdate(6);

    ExpectWindowSizeUpdateWithComplete(5);
    ContentMessageReceivedAndObserverNotified({ 0xf9, 0xf9, '1', '2', '3', '4', 0xf9, 0xfe });
    reader = connection->ReceiveStream();
    EXPECT_EQ(6, reader->Available());
    EXPECT_EQ(std::vector<uint8_t>({ 0xf9, '1', '2', '3', '4', 0xfe }), reader->ExtractContiguousRange(6));
}

TEST_F(ConnectionSerialTest, scrap_bytes_received_after_initialization_are_scrapped_and_connection_is_reset)
{
    Initialization();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 'a' });
    ExpectAndCompleteWindowSizeRequest();

    Initialization();
}

class ConnectionSerialTestHeaderBytesTest
    : public ConnectionSerialTest
    , public testing::WithParamInterface<uint8_t>
{};

TEST_P(ConnectionSerialTestHeaderBytesTest, unescaped_header_bytes_received_midcontent_message_are_discarded_and_connection_state_is_reset1)
{
    Initialization();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfd });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ GetParam() });

    ExpectAndCompleteWindowSizeRequest();
}

TEST_P(ConnectionSerialTestHeaderBytesTest, unescaped_header_bytes_received_midcontent_message_are_discarded_and_connection_state_is_reset2)
{
    Initialization();

    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xfd });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 5 });

    serialCommunication.dataReceived(std::vector<uint8_t>{ '1' });
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    EXPECT_CALL(observer, DataReceived());
    ExpectAndCompleteUpdate(4);
    ExecuteAllActions();
    serialCommunication.dataReceived(std::vector<uint8_t>{ 0xf9 });
    serialCommunication.dataReceived(std::vector<uint8_t>{ GetParam() });

    ExpectAndCompleteWindowSizeRequest();
}

INSTANTIATE_TEST_SUITE_P(HeaderBytes, ConnectionSerialTestHeaderBytesTest, testing::Values(0xfa, 0xfb, 0xfc, 0xfd, 0xfe));

TEST_F(ConnectionSerialTest, reset_connection_during_a_content_send)
{
    Initialization();

    GetAndWriteToSendStream({ '1' });

    EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>({ 0xfd, static_cast<uint8_t>(1 >> 8), static_cast<uint8_t>(1) })))
        .WillOnce(testing::InvokeWithoutArgs([this]()
    {
        WindowSizeRequestReceived();
    }));
    EXPECT_CALL(serialCommunication, SendDataMock(std::vector<uint8_t>({ '1' })));
    ExecuteAllActions();
    serialCommunication.actionOnCompletion();
    ExpectWindowSizeRequest();
    serialCommunication.actionOnCompletion();

    ExecuteAllActions();
    serialCommunication.actionOnCompletion();

    Initialization();
}

//Test if observer.DataReceived is called again when receiveStream is alive and new data arrives
//Test receiving all init messages when in sending
//Test reset_connection_during_a_update_send