#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "services/util/MessageCommunicationWindowed.hpp"
#include "services/util/test_doubles/MessageCommunicationMock.hpp"
#include "gmock/gmock.h"
#include <deque>

class MessageCommunicationWindowedTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    MessageCommunicationWindowedTest()
    {
        EXPECT_CALL(base, MessageSize(testing::_)).WillRepeatedly(testing::Invoke([](std::size_t size)
            {
                return size + size / 254 + 2;
            }));
    }

    void SendMessageStreamAvailableWithWriter(const std::vector<uint8_t>& expected)
    {
        assert(sentData.empty());
        assert(expectedMessage.empty());
        expectedMessage = expected;
        writer.OnAllocatable([this]()
            {
                auto expectedMessageCopy = expectedMessage;
                expectedMessage.clear();
                base.GetObserver().MessageSent(sentData.size() + sentData.size() / 254 + 2);
                EXPECT_EQ(expectedMessageCopy, sentData);
                sentData.clear();
            });
        base.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));
    }

    void ReceiveMessage(const std::vector<uint8_t>& data)
    {
        infra::SharedOptional<infra::StdVectorInputStreamReader::WithStorage> reader;
        base.GetObserver().ReceivedMessage(reader.Emplace(infra::inPlace, data));
    }

    void ReceiveInitRequest(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceiveMessage(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ReceiveInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceiveMessage(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ReceiveReleaseWindow(uint16_t availableWindow)
    {
        ReceiveMessage(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ExpectRequestSendMessageForInit(uint16_t availableWindow)
    {
        EXPECT_CALL(base, RequestSendMessage(3)).WillOnce(testing::Invoke([this, availableWindow](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
            }));
    }

    void ExpectRequestSendMessageForInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(base, RequestSendMessage(3)).WillOnce(testing::Invoke([this, availableWindow](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
            }));
    }

    void ExpectRequestSendMessageForReleaseWindow(uint16_t releasedSize)
    {
        EXPECT_CALL(base, RequestSendMessage(3)).WillOnce(testing::Invoke([this, releasedSize](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(releasedSize).Vector());
            }));
    }

    void ExpectRequestSendMessageForMessage(uint16_t size, const std::vector<uint8_t>& expected)
    {
        EXPECT_CALL(base, RequestSendMessage(size)).WillOnce(testing::Invoke([this, expected](uint16_t size)
            {
                SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(4)(expected).Vector());
            }));
    }

    void ExpectSendMessageStreamAvailable(const std::vector<uint8_t>& data)
    {
        EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([data](infra::SharedPtr<infra::StreamWriter>&& writer)
            {
                infra::DataOutputStream::WithErrorPolicy stream(*writer);
                stream << infra::MakeRange(data);
            }));
    }

    void ExpectSendMessageStreamAvailableAndSaveWriter()
    {
        EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_)).WillOnce(testing::SaveArg<0>(&savedWriter));
    }

    void ExpectReceivedMessage(const std::vector<uint8_t>& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::DataInputStream::WithErrorPolicy stream(*reader);
                std::vector<uint8_t> data(stream.Available(), 0);
                stream >> infra::MakeRange(data);

                EXPECT_EQ(expected, data);
            }));
    }

    testing::StrictMock<services::MessageCommunicationEncodedMock> base;
    std::vector<uint8_t> expectedMessage;
    std::vector<uint8_t> sentData;
    infra::NotifyingSharedOptional<infra::StdVectorOutputStreamWriter> writer;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(base, MessageSize(3)).WillOnce(testing::Return(5));
            ExpectRequestSendMessageForInit(16);
        } };
    services::MessageCommunicationWindowed communication{ base, 16 };
    testing::StrictMock<services::MessageCommunicationObserverMock> observer{ communication };
    infra::SharedPtr<infra::StreamWriter> savedWriter;
};

TEST_F(MessageCommunicationWindowedTest, MaxSendMessageSize)
{
    ReceiveInitResponse(8);
    EXPECT_CALL(base, MaxSendMessageSize()).WillOnce(testing::Return(16));
    EXPECT_EQ(10, communication.MaxSendMessageSize());
}

TEST_F(MessageCommunicationWindowedTest, send_message_after_initialized)
{
    ReceiveInitResponse(12);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);
}

TEST_F(MessageCommunicationWindowedTest, message_waits_until_window_is_freed)
{
    ReceiveInitResponse(6);

    communication.RequestSendMessage(4);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveReleaseWindow(6);
}

TEST_F(MessageCommunicationWindowedTest, long_message_waits_until_window_is_freed_taking_into_account_cobs_overhead)
{
    ReceiveInitResponse(261);

    // A message of size 253 plus one byte for the operation may consist of 254 zeros, and therefore need one extra COBS overhead byte.
    // Total window available needs therefore to be 254 + 1 (one extra COBS byte) + 2 (normal COBS byte and closing 0) + 5 (safeguard for another window release)
    communication.RequestSendMessage(253);

    ExpectRequestSendMessageForMessage(254, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveReleaseWindow(1);
}

TEST_F(MessageCommunicationWindowedTest, exact_used_window_size_is_consumed_by_message)
{
    auto longMessage = infra::ConstructBin().Repeat(253 * 7, 0).Vector();

    // Exactly enough for any message of size longMessage plus safeguard for a window release
    ReceiveInitResponse(254 * 7 + 1 + 7 + 2 + 5);

    // Sending longMessage, for which no extra COBS overhead bytes were necessary, results in 7 + 5 window still available
    ExpectRequestSendMessageForMessage(longMessage.size() + 1, longMessage);
    ExpectSendMessageStreamAvailable(longMessage);
    communication.RequestSendMessage(longMessage.size());

    // Now prove that enough window is still available by sending another message
    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);
}

TEST_F(MessageCommunicationWindowedTest, send_message_while_initializing_waits_for_initialized)
{
    communication.RequestSendMessage(4);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveInitResponse(12);
}

TEST_F(MessageCommunicationWindowedTest, request_sending_new_message_while_previous_is_still_processing)
{
    ReceiveInitResponse(20);

    EXPECT_CALL(base, RequestSendMessage(5));
    communication.RequestSendMessage(4);

    EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([this](infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            infra::DataOutputStream::WithErrorPolicy stream(*writer);
            const std::vector<uint8_t>& data = { 1, 2, 3, 4 };
            stream << infra::MakeRange(data);

            writer = nullptr;
            communication.RequestSendMessage(2);
        }));

    EXPECT_CALL(base, RequestSendMessage(3));
    SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    ExpectSendMessageStreamAvailable({ 5, 6 });
    SendMessageStreamAvailableWithWriter(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, receive_message_after_initialized)
{
    ReceiveInitResponse(12);

    ExpectReceivedMessage(infra::ConstructBin()("abcd").Vector());
    ExpectRequestSendMessageForReleaseWindow(6);
    ReceiveMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());
}

TEST_F(MessageCommunicationWindowedTest, release_window_packet_waits_for_window_available)
{
    ReceiveInitResponse(0);

    ExpectReceivedMessage(infra::ConstructBin()("abcd").Vector());
    ReceiveMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    ExpectRequestSendMessageForReleaseWindow(6);
    ReceiveReleaseWindow(5);
}

TEST_F(MessageCommunicationWindowedTest, received_message_before_initialized_is_discarded)
{
    ReceiveMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    ReceiveInitResponse(12);
}

TEST_F(MessageCommunicationWindowedTest, received_release_window_before_initialized_is_discarded)
{
    communication.RequestSendMessage(4);
    ReceiveReleaseWindow(6);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveInitResponse(12);
}

TEST_F(MessageCommunicationWindowedTest, handle_init_request_after_initialization)
{
    ReceiveInitResponse(8);

    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);
}

TEST_F(MessageCommunicationWindowedTest, init_response_consumes_window)
{
    ReceiveInitResponse(16);

    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(5 + 7 + 7); // window for two messages and saving for releaseWindow

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);

    communication.RequestSendMessage(4);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveReleaseWindow(5); // release window consumed by initResponse
}

TEST_F(MessageCommunicationWindowedTest, release_window_consumes_window)
{
    ReceiveInitResponse(5); // window for one release window

    ExpectReceivedMessage(infra::ConstructBin()("abcd").Vector());
    ExpectRequestSendMessageForReleaseWindow(6);
    ReceiveMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    ExpectReceivedMessage(infra::ConstructBin()("abcd").Vector());
    ReceiveMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    ExpectRequestSendMessageForReleaseWindow(6);
    ReceiveReleaseWindow(5); // release window consumed by release window
}

TEST_F(MessageCommunicationWindowedTest, received_init_request_while_sending_message_finishes_message_then_sends_init_response)
{
    // build
    ReceiveInitResponse(16);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailableAndSaveWriter();

    communication.RequestSendMessage(4);

    // operate
    ReceiveInitRequest(12);

    ExpectRequestSendMessageForInitResponse(16);

    {
        std::vector<uint8_t> data{ 1, 2, 3, 4 };
        infra::DataOutputStream::WithErrorPolicy stream(*savedWriter);
        stream << infra::MakeRange(data);
        savedWriter = nullptr;
    };
}

TEST_F(MessageCommunicationWindowedTest, increase_window_while_sending)
{
    // build
    ReceiveInitResponse(12);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    ReceiveReleaseWindow(6);

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessageForMessage(3, { 5, 6 });
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
}

TEST_F(MessageCommunicationWindowedTest, init_response_while_sending)
{
    // build
    ReceiveInitResponse(12);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    ReceiveInitResponse(10);

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessageForMessage(3, { 5, 6 });
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
}

TEST_F(MessageCommunicationWindowedTest, received_init_request_while_sending_init)
{
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, init_response_while_sending_init_is_ignored)
{
    ReceiveInitResponse(8);

    ReceiveInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, release_window_while_sending_init_is_ignored)
{
    ReceiveReleaseWindow(4);

    ReceiveInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, requesting_message_while_sending_init_response)
{
    // build
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);

    // operate
    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);
}

TEST_F(MessageCommunicationWindowedTest, init_request_while_sending_init_response_results_in_new_init_response)
{
    // build
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);

    // operate
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);
}

TEST_F(MessageCommunicationWindowedTest, release_window_while_sending_init_response_is_ignored)
{
    // build
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);

    // operate
    ReceiveReleaseWindow(4);
}
