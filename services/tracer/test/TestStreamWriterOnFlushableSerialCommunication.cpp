#include "hal/interfaces/SerialCommunication.hpp"
#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "services/tracer/StreamWriterOnFlushableSerialCommunication.hpp"
#include "gmock/gmock.h"

class StreamWriterOnFlushableSerialCommunicationTest
    : public testing::Test
{
public:
    StreamWriterOnFlushableSerialCommunicationTest()
        : streamWriter(communication, flushableCommunication)
    {}

    testing::StrictMock<hal::SerialCommunicationMock> communication;
    testing::StrictMock<hal::FlushableMock> flushableCommunication;
    services::StreamWriterOnFlushableSerialCommunication::WithStorage<4> streamWriter;
    infra::StreamErrorPolicy errorPolicy;
};

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, Insert_sends_one_element_on_communication)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, no_communication_when_buffer_is_empty)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);
    communication.actionOnCompletion();
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, Insert_sends_range_on_communication)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5, 8 }));
    streamWriter.Insert(std::array<uint8_t, 2>{ 5, 8 }, errorPolicy);
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, second_Insert_holds_if_communication_not_complete)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(8)), errorPolicy);
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, second_Insert_is_sent_after_communication_is_done)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(8)), errorPolicy);

    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 8 }));
    communication.actionOnCompletion();
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, on_overflow_rest_is_discarded)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2, 3, 4 }));
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2, 3, 4, 5 }, errorPolicy);
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, available_returns_max)
{
    EXPECT_EQ(std::numeric_limits<size_t>::max(), streamWriter.Available());
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, insert_after_flush_on_empty_buffer_sends_data)
{
    streamWriter.Flush();

    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, flush_completes_pending_communication_when_FlushSendBuffer_supported)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);

    EXPECT_CALL(flushableCommunication, Flush()).WillOnce([this]()
        {
            communication.actionOnCompletion();
        });
    streamWriter.Flush();
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, stale_completion_after_flush_does_nothing)
{
    infra::Function<void()> savedCallback;
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }))
        .WillOnce([this, &savedCallback]()
            {
                savedCallback = communication.actionOnCompletion.Clone();
            });
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);

    // Flush completes the in-progress send synchronously via FlushSendBuffer
    EXPECT_CALL(flushableCommunication, Flush()).WillOnce([this]()
        {
            communication.actionOnCompletion();
        });
    streamWriter.Flush();

    // The stale callback from the event queue fires — this should be a no-op
    savedCallback();

    // Verify state is clean: a new Insert should immediately trigger SendData
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 6 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(6)), errorPolicy);
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, stale_completion_does_not_interfere_with_new_transaction)
{
    infra::Function<void()> savedCallback;
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 5 }))
        .WillOnce([this, &savedCallback]()
            {
                savedCallback = communication.actionOnCompletion.Clone();
            });
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(5)), errorPolicy);

    // Flush completes the in-progress send synchronously via Flush
    EXPECT_CALL(flushableCommunication, Flush()).WillOnce([this]()
        {
            communication.actionOnCompletion();
        });
    streamWriter.Flush();

    // A new transaction starts; savedCallback now belongs to the previous transaction
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 8 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(8)), errorPolicy);

    // The stale callback fires — it should be ignored because the transaction ID no longer matches
    savedCallback();
    // Verify the new transaction is still in progress: Insert queues data without triggering SendData
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(6)), errorPolicy);

    // The real completion for the new transaction fires, flushing the queued byte
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 6 }));
    communication.actionOnCompletion();
}
