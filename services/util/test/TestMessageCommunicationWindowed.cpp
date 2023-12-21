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
    void EmplaceWriterAndMakeAvailable(const infra::Function<void()>& onAllocatable = []() {})
    {
        sentData.emplace_back();
        writer.OnAllocatable(onAllocatable);
        base.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData.back()));
    }

    void ReceivedMessage(const std::vector<uint8_t>& data)
    {
        infra::SharedOptional<infra::StdVectorInputStreamReader::WithStorage> reader;
        base.GetObserver().ReceivedMessage(reader.Emplace(infra::inPlace, data));
    }

    void SendInitRequest(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceivedMessage(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void SendInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(observer, Initialized());
        ReceivedMessage(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void SendReleaseWindow(uint16_t availableWindow)
    {
        ReceivedMessage(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void FinishInitialization(uint16_t availableWindow)
    {
        ExpectSentData(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(16).Vector());
        SendInitResponse(availableWindow);
    }

    void ExpectSendInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(base, RequestSendMessage(3)).WillOnce(testing::Invoke([this, availableWindow](uint16_t size)
            {
                EmplaceWriterAndMakeAvailable();
            }));
    }

    void ExpectRequestSendMessage(uint16_t size)
    {
        EXPECT_CALL(base, RequestSendMessage(size)).WillOnce(testing::Invoke([this](uint16_t size)
            {
                EmplaceWriterAndMakeAvailable();
            }));
    }

    void ExpectSentData(const std::vector<uint8_t>& expected)
    {
        EXPECT_EQ(expected, sentData.front());
        sentData.pop_front();
    }

    void ExpectReceivedInitResponse(uint16_t availableWindow)
    {
        EXPECT_EQ(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector(), sentData.front());
        sentData.pop_front();
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

    void ExpectSendReleaseWindow(uint16_t releasedSize)
    {
        EXPECT_CALL(base, RequestSendMessage(3)).WillOnce(testing::Invoke([this, releasedSize](uint16_t size)
            {
                EmplaceWriterAndMakeAvailable([this, releasedSize]()
                    {
                        EXPECT_EQ(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(releasedSize).Vector(), sentData.front());
                        sentData.pop_front();
                    });
            }));
    }

    testing::StrictMock<services::MessageCommunicationMock> base;
    std::deque<std::vector<uint8_t>> sentData;
    infra::NotifyingSharedOptional<infra::StdVectorOutputStreamWriter> writer;
    infra::Execute execute{ [this]()
        {
            ExpectRequestSendMessage(3);
        } };
    services::MessageCommunicationWindowed communication{ base, 16 };
    testing::StrictMock<services::MessageCommunicationObserverMock> observer{ communication };
    infra::SharedPtr<infra::StreamWriter> savedWriter;
};

TEST_F(MessageCommunicationWindowedTest, construction)
{
    FinishInitialization(4);
    EXPECT_CALL(base, MaxSendMessageSize()).WillOnce(testing::Return(20));
    EXPECT_EQ(19, communication.MaxSendMessageSize());
}

TEST_F(MessageCommunicationWindowedTest, send_message_after_initialized)
{
    FinishInitialization(6);

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);
    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, message_waits_until_window_is_freed)
{
    FinishInitialization(2);

    communication.RequestSendMessage(4);

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    SendReleaseWindow(4);
    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, send_message_while_initializing_waits_for_initialized)
{
    communication.RequestSendMessage(4);

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    FinishInitialization(6);

    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, request_sending_new_message_while_previous_is_still_processing)
{
    FinishInitialization(12);

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
    EmplaceWriterAndMakeAvailable();

    ExpectSendMessageStreamAvailable({ 5, 6 });
    EmplaceWriterAndMakeAvailable();

    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, receive_message_after_initialized)
{
    FinishInitialization(6);

    ExpectReceivedMessage(infra::ConstructBin()("abcd").Vector());
    ExpectSendReleaseWindow(6);
    ReceivedMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());
}

TEST_F(MessageCommunicationWindowedTest, received_message_before_initialized_is_discarded)
{
    ReceivedMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    FinishInitialization(6);
}

TEST_F(MessageCommunicationWindowedTest, received_release_window_before_initialized_is_discarded)
{
    communication.RequestSendMessage(4);
    SendReleaseWindow(6);

    ExpectSentData(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(16).Vector());

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    SendInitResponse(6);
}

TEST_F(MessageCommunicationWindowedTest, handle_init_request_after_initialization)
{
    FinishInitialization(4);

    ExpectSendInitResponse(16);
    SendInitRequest(4);
    ExpectReceivedInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, received_init_request_while_sending_message_finishes_message_then_sends_init_response)
{
    // build
    FinishInitialization(12);

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailableAndSaveWriter();

    communication.RequestSendMessage(4);

    // operate
    SendInitRequest(4);

    ExpectSendInitResponse(16);

    {
        std::vector<uint8_t> data{ 1, 2, 3, 4 };
        infra::DataOutputStream::WithErrorPolicy stream(*savedWriter);
        stream << infra::MakeRange(data);
        savedWriter = nullptr;
    };

    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
    ExpectReceivedInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, increase_window_while_sending)
{
    // build
    FinishInitialization(6);

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    SendReleaseWindow(4);

    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessage(3);
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, init_response_while_sending)
{
    // build
    FinishInitialization(6);

    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    SendInitResponse(4);

    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    // Send a new message that uses the newly announced window
    ExpectRequestSendMessage(3);
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, received_init_request_while_sending_init)
{
    ExpectSendInitResponse(16);
    SendInitRequest(4);

    FinishInitialization(12);

    ExpectReceivedInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, init_response_while_sending_init_is_ignored)
{
    SendInitResponse(4);

    FinishInitialization(12);
}

TEST_F(MessageCommunicationWindowedTest, release_window_while_sending_init_is_ignored)
{
    SendReleaseWindow(4);

    FinishInitialization(12);
}

TEST_F(MessageCommunicationWindowedTest, requesting_message_while_sending_init_response)
{
    // build
    ExpectSendInitResponse(16);
    SendInitRequest(4);

    FinishInitialization(12);

    // operate
    ExpectRequestSendMessage(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    communication.RequestSendMessage(4);

    ExpectReceivedInitResponse(16);
    ExpectSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(MessageCommunicationWindowedTest, init_request_while_sending_init_response_results_in_new_init_response)
{
    // build
    ExpectSendInitResponse(16);
    SendInitRequest(4);

    FinishInitialization(12);

    // operate
    ExpectSendInitResponse(16);
    SendInitRequest(4);

    ExpectReceivedInitResponse(16);

    ExpectReceivedInitResponse(16);
}

TEST_F(MessageCommunicationWindowedTest, release_window_while_sending_init_response_is_ignored)
{
    // build
    ExpectSendInitResponse(16);
    SendInitRequest(4);

    FinishInitialization(12);

    // operate
    SendReleaseWindow(4);

    ExpectReceivedInitResponse(16);
}
