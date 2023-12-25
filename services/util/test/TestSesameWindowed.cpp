#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"
#include <deque>

class SesameWindowedTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    SesameWindowedTest()
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

    void ReceivePacket(const std::vector<uint8_t>& data)
    {
        base.GetObserver().ReceivedMessage(reader.Emplace(infra::inPlace, data), data.size() + data.size() / 254 + 2);
    }

    void ReceiveInitRequest(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceivePacket(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ReceiveInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceivePacket(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ReceiveReleaseWindow(uint16_t availableWindow)
    {
        ReceivePacket(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void ReceiveMessage(const std::string& text)
    {
        ReceivePacket(infra::ConstructBin()(4)(text).Vector());
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

    void ExpectReceivedMessage(const std::string& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::DataInputStream::WithErrorPolicy stream(*reader);
                std::string text(stream.Available(), 0);
                stream >> infra::ByteRange(reinterpret_cast<uint8_t*>(text.data()), reinterpret_cast<uint8_t*>(text.data() + text.size()));

                EXPECT_EQ(expected, text);
            }));
    }

    void ExpectReceivedMessageAndSaveReader(const std::string& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_)).WillOnce(testing::SaveArg<0>(&savedReader));
    }

    testing::StrictMock<services::SesameEncodedMock> base;
    std::vector<uint8_t> expectedMessage;
    std::vector<uint8_t> sentData;
    infra::NotifyingSharedOptional<infra::StdVectorOutputStreamWriter> writer;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(base, MaxSendMessageSize()).WillOnce(testing::Return(16));
            EXPECT_CALL(base, MessageSize(3)).WillOnce(testing::Return(5));
            ExpectRequestSendMessageForInit(16);
        } };
    services::SesameWindowed communication{ base };
    testing::StrictMock<services::SesameObserverMock> observer{ communication };
    infra::SharedPtr<infra::StreamWriter> savedWriter;
    infra::SharedOptional<infra::StdVectorInputStreamReader::WithStorage> reader;
    infra::SharedPtr<infra::StreamReaderWithRewinding> savedReader;
};

TEST_F(SesameWindowedTest, MaxSendMessageSize)
{
    // When the cobs layer is able to send a 16 byte message, then 18 bytes (cobs start plus delimiter) are available
    ReceiveInitResponse(16);
    // 4 bytes in a message expands to 1 (cobs) + 1 (operation) + 3 (message) + 1 (delimiter) = 6
    // Two of these messages plus one release window amount to 6 + 6 + 5 = 17, which is under the limit of the 18 bytes buffer of cobs
    EXPECT_EQ(3, communication.MaxSendMessageSize());
}

TEST_F(SesameWindowedTest, send_message_after_initialized)
{
    ReceiveInitResponse(12);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);
}

TEST_F(SesameWindowedTest, message_waits_until_window_is_freed)
{
    ReceiveInitResponse(6);

    communication.RequestSendMessage(4);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveReleaseWindow(6);
}

TEST_F(SesameWindowedTest, long_message_waits_until_window_is_freed_taking_into_account_cobs_overhead)
{
    ReceiveInitResponse(261);

    // A message of size 253 plus one byte for the operation may consist of 254 zeros, and therefore need one extra COBS overhead byte.
    // Total window available needs therefore to be 254 + 1 (one extra COBS byte) + 2 (normal COBS byte and closing 0) + 5 (safeguard for another window release)
    communication.RequestSendMessage(253);

    ExpectRequestSendMessageForMessage(254, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveReleaseWindow(1);
}

TEST_F(SesameWindowedTest, exact_used_window_size_is_consumed_by_message)
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

TEST_F(SesameWindowedTest, send_message_while_initializing_waits_for_initialized)
{
    communication.RequestSendMessage(4);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveInitResponse(12);
}

TEST_F(SesameWindowedTest, request_sending_new_message_while_previous_is_still_processing)
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

TEST_F(SesameWindowedTest, receive_message_after_initialized)
{
    ReceiveInitResponse(12);

    ExpectReceivedMessage("abcd");
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveMessage("abcd");
}

TEST_F(SesameWindowedTest, release_window_packet_waits_for_window_available)
{
    ReceiveInitResponse(0);

    ExpectReceivedMessage("abcd");
    ReceiveMessage("abcd");

    ExpectRequestSendMessageForReleaseWindow(17);
    ReceiveReleaseWindow(5);
}

TEST_F(SesameWindowedTest, received_message_before_initialized_is_discarded)
{
    ReceiveMessage("abcd");

    ReceiveInitResponse(12);
}

TEST_F(SesameWindowedTest, received_release_window_before_initialized_is_discarded)
{
    communication.RequestSendMessage(4);
    ReceiveReleaseWindow(6);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    ReceiveInitResponse(12);
}

TEST_F(SesameWindowedTest, handle_init_request_after_initialization)
{
    ReceiveInitResponse(8);

    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);
}

TEST_F(SesameWindowedTest, init_response_consumes_window)
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

TEST_F(SesameWindowedTest, release_window_consumes_window)
{
    ReceiveInitResponse(5); // window for one release window

    ExpectReceivedMessage("abcd");
    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveMessage("abcd");

    ExpectReceivedMessage("abcd");
    ReceiveMessage("abcd");

    ExpectRequestSendMessageForReleaseWindow(12);
    ReceiveReleaseWindow(5); // release window consumed by release window
}

TEST_F(SesameWindowedTest, received_init_request_while_sending_message_finishes_message_then_sends_init_response)
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

TEST_F(SesameWindowedTest, increase_window_while_sending)
{
    // build
    ReceiveInitResponse(12);

    ExpectRequestSendMessageForMessage(5, { 1, 2, 3, 4 });
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);

    // operate
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveReleaseWindow(11);

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessageForMessage(3, { 5, 6 });
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
}

TEST_F(SesameWindowedTest, init_response_while_sending)
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

TEST_F(SesameWindowedTest, received_init_request_while_sending_init)
{
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);
}

TEST_F(SesameWindowedTest, init_response_while_sending_init_is_ignored)
{
    ReceiveInitResponse(8);

    ReceiveInitResponse(16);
}

TEST_F(SesameWindowedTest, release_window_while_sending_init_is_ignored)
{
    ReceiveReleaseWindow(4);

    ReceiveInitResponse(16);
}

TEST_F(SesameWindowedTest, requesting_message_while_sending_init_response)
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

TEST_F(SesameWindowedTest, init_request_while_sending_init_response_results_in_new_init_response)
{
    // build
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);

    // operate
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);
}

TEST_F(SesameWindowedTest, release_window_while_sending_init_response_is_ignored)
{
    // build
    ExpectRequestSendMessageForInitResponse(16);
    ReceiveInitRequest(8);

    ReceiveInitResponse(16);

    // operate
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveReleaseWindow(4);
}

TEST_F(SesameWindowedTest, no_window_release_after_small_message)
{
    // build
    ReceiveInitResponse(12);
    ExpectReceivedMessage("ab");
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveMessage("ab");

    // operate
    ExpectReceivedMessage("ab");
    // ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveMessage("ab");
}

TEST_F(SesameWindowedTest, no_window_release_after_just_release_window)
{
    // build
    ReceiveInitResponse(12);
    ExpectReceivedMessage("ab");
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveMessage("ab");

    // operate
    // ExpectRequestSendMessageForReleaseWindow(5);
    ReceiveReleaseWindow(50);
}

TEST_F(SesameWindowedTest, window_release_after_second_release_window)
{
    // build
    ReceiveInitResponse(12);
    ExpectReceivedMessage("ab");
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveMessage("ab");

    // operate
    ExpectRequestSendMessageForReleaseWindow(10);
    ReceiveReleaseWindow(50);
    ReceiveReleaseWindow(50);
}

TEST_F(SesameWindowedTest, window_is_released_after_message_has_been_processed)
{
    ReceiveInitResponse(12);

    ExpectReceivedMessageAndSaveReader("abcd");
    ReceiveMessage("abcd");

    ExpectRequestSendMessageForReleaseWindow(12);
    savedReader = nullptr;
}
