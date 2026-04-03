#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "services/tracer/StreamWriterOnFlushableSerialCommunication.hpp"
#include "services/util/test_doubles/FlushableMock.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class FlushableStreamWriterAccessor
    : public services::StreamWriterOnFlushableSerialCommunication
{
public:
    template<std::size_t StorageSize>
    using WithStorage = infra::WithStorage<FlushableStreamWriterAccessor, std::array<uint8_t, StorageSize>>;
    using StreamWriterOnFlushableSerialCommunication::StreamWriterOnFlushableSerialCommunication;

    auto IsCurrentlySendingBytes() const
    {
        return IsCurrentlySending();
    }
};

class StreamWriterOnFlushableSerialCommunicationTest
    : public testing::Test

{
public:
    StreamWriterOnFlushableSerialCommunicationTest()
        : streamWriter(communication, flushable)
    {}

    testing::StrictMock<hal::SerialCommunicationMock> communication;
    testing::StrictMock<services::FlushableMock> flushable;
    FlushableStreamWriterAccessor::WithStorage<4> streamWriter;
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

    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2 }));
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2 }, errorPolicy);

    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(true));
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, flush_completes_pending_communication_when_FlushSendBuffer_supported)
{
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2 }));
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2 }, errorPolicy);

    EXPECT_CALL(flushable, Flush());
    streamWriter.Flush();

    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));
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

    EXPECT_CALL(flushable, Flush());
    streamWriter.Flush();

    // The stale callback from the event queue fires — this should be a no-op
    savedCallback();

    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, stale_completion_does_not_interfere_with_new_transaction)
{
    infra::Function<void()> savedCallback;
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2 }))
        .WillOnce([this, &savedCallback]()
            {
                savedCallback = communication.actionOnCompletion.Clone();
            });
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2 }, errorPolicy);

    EXPECT_CALL(flushable, Flush());
    streamWriter.Flush();
    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));

    // A new transaction starts; the stray queued event (savedCallback) now belongs to the previous transaction
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 8 }));
    streamWriter.Insert(infra::MakeByteRange(static_cast<uint8_t>(8)), errorPolicy);
    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(true));
    // Stray callback is ignored
    savedCallback();
    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(true));

    // The real completion for the new transaction fires, flushing the queued byte
    communication.actionOnCompletion();
    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, flush_drains_all_queued_data_across_multiple_sends)
{
    // Insert A: starts sending immediately
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2 }));
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2 }, errorPolicy);

    // Insert B while A is still in-flight: queued behind A
    streamWriter.Insert(std::vector<uint8_t>{ 3, 4 }, errorPolicy);

    // Flush() must drain both chunks; flushable.Flush() should be called twice
    // (once per in-flight chunk as the while-loop iterates)
    EXPECT_CALL(flushable, Flush()).Times(2);
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 3, 4 }));
    streamWriter.Flush();

    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, flush_tolerates_completion_callback_fired_by_flushable)
{
    // This test verifies that if flushable.Flush() triggers the serial completion
    // callback (which calls OnCommunicationDone), the subsequent manual
    // OnCommunicationDone(transactionId) in Flush() is a no-op thanks to the
    // transactionId guard -- no double-pop / data corruption occurs.
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2 }));
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2 }, errorPolicy);

    // When flushable.Flush() is called, simulate the underlying driver
    // completing the send synchronously (i.e., fires the actionOnCompletion callback)
    EXPECT_CALL(flushable, Flush())
        .WillOnce([this]()
            {
                communication.actionOnCompletion();
            });
    streamWriter.Flush();

    // Despite the double-completion attempt, state must be clean
    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));
}

TEST_F(StreamWriterOnFlushableSerialCommunicationTest, flush_tolerates_completion_callback_during_multi_transaction_drain)
{
    // Insert A: starts sending immediately
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 1, 2 }));
    streamWriter.Insert(std::vector<uint8_t>{ 1, 2 }, errorPolicy);

    // Insert B while A is still in-flight
    streamWriter.Insert(std::vector<uint8_t>{ 3, 4 }, errorPolicy);

    // For each chunk, flushable.Flush() triggers the serial completion callback
    // synchronously. Flush() should still drain everything safely.
    EXPECT_CALL(communication, SendDataMock(std::vector<uint8_t>{ 3, 4 }));
    EXPECT_CALL(flushable, Flush())
        .Times(2)
        .WillRepeatedly([this]()
            {
                communication.actionOnCompletion();
            });
    streamWriter.Flush();

    EXPECT_THAT(streamWriter.IsCurrentlySendingBytes(), testing::Eq(false));
}
