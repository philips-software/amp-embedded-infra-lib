#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "services/network/MessageCommunication.hpp"
#include "gmock/gmock.h"
#include <deque>

namespace
{
    class MessageCommunicationReceiveOnInterruptMock
        : public services::MessageCommunicationReceiveOnInterrupt
    {
    public:
        MOCK_METHOD2(SendMessageStream, infra::SharedPtr<infra::StreamWriter>(uint16_t size, const infra::Function<void(uint16_t size)>& onSent));
        MOCK_CONST_METHOD0(MaxSendMessageSize, std::size_t());
    };

    class MessageCommunicationObserverMock
        : public services::MessageCommunicationObserver
    {
    public:
        using services::MessageCommunicationObserver::MessageCommunicationObserver;

        MOCK_METHOD1(SendMessageStreamAvailable, void(infra::SharedPtr<infra::StreamWriter>&& writer));
        MOCK_METHOD1(ReceivedMessage, void(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader));
    };
}

class WindowedMessageCommunicationTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    infra::SharedPtr<infra::StreamWriter> EmplaceWriter(const infra::Function<void()>& onAllocatable = []() {})
    {
        sentData.emplace_back();
        writer.OnAllocatable(onAllocatable);
        return writer.Emplace(sentData.back());
    }

    void ReceivedMessage(const std::vector<uint8_t>& data)
    {
        infra::StdVectorInputStreamReader::WithStorage reader(infra::inPlace, data);
        base.GetObserver().ReceivedMessageOnInterrupt(reader);
    }

    void SendInitRequest(uint16_t availableWindow)
    {
        ReceivedMessage(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void SendInitResponse(uint16_t availableWindow)
    {
        ReceivedMessage(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void SendReleaseWindow(uint16_t availableWindow)
    {
        ReceivedMessage(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector());
    }

    void FinishInitialization(uint16_t availableWindow)
    {
        OnSentData(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(16).Vector());
        SendInitResponse(availableWindow);
    }

    void ExpectSendInitResponse(uint16_t availableWindow)
    {
        EXPECT_CALL(base, SendMessageStream(3, testing::_)).WillOnce(testing::Invoke([this, availableWindow](uint16_t size, const infra::Function<void(uint16_t size)>& onSent)
        {
            this->onSent = onSent;
            return EmplaceWriter();
        }));
    }

    void ExpectSendMessageStream(uint16_t size)
    {
        EXPECT_CALL(base, SendMessageStream(size, testing::_)).WillOnce(testing::Invoke([this](uint16_t size, const infra::Function<void(uint16_t size)>& onSent)
        {
            this->onSent = onSent;
            return EmplaceWriter();
        }));
    }

    void OnSentData(const std::vector<uint8_t>& expected)
    {
        this->onSent(static_cast<uint16_t>(expected.size() - 1));
        EXPECT_EQ(expected, sentData.front());
        sentData.pop_front();
    }

    void OnSentInitResponse(uint16_t availableWindow)
    {
        this->onSent(3);
        EXPECT_EQ(infra::ConstructBin().Value<uint8_t>(2).Value<infra::LittleEndian<uint16_t>>(availableWindow).Vector(), sentData.front());
        sentData.pop_front();
    }

    void ExpectSendMessageStreamAvailable(const std::vector<uint8_t>& data)
    {
        EXPECT_CALL(observer, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([data](infra::SharedPtr<infra::StreamWriter>& writer)
        {
            infra::DataOutputStream::WithErrorPolicy stream(*writer);
            stream << infra::MakeRange(data);
        }));
    }

    void ExpectReceivedMessage(const std::vector<uint8_t>& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([expected](infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
        {
            infra::DataInputStream::WithErrorPolicy stream(*reader);
            std::vector<uint8_t> data(stream.Available(), 0);
            stream >> infra::MakeRange(data);

            EXPECT_EQ(expected, data);
        }));
    }

    void ExpectSendReleaseWindow(uint16_t releasedSize)
    {
        EXPECT_CALL(base, SendMessageStream(3, testing::_)).WillOnce(testing::Invoke([this, releasedSize](uint16_t size, const infra::Function<void(uint16_t size)>& onSent)
        {
            this->onSent = onSent;
            return EmplaceWriter([this, releasedSize]()
            {
                this->onSent(3);
                EXPECT_EQ(infra::ConstructBin().Value<uint8_t>(3).Value<infra::LittleEndian<uint16_t>>(releasedSize).Vector(), sentData.front());
                sentData.pop_front();
            });
        }));
    }

    testing::StrictMock<MessageCommunicationReceiveOnInterruptMock> base;
    std::deque<std::vector<uint8_t>> sentData;
    infra::NotifyingSharedOptional<infra::StdVectorOutputStreamWriter> writer;
    infra::AutoResetFunction<void(uint16_t size)> onSent;
    infra::Execute execute{ [this]() { ExpectSendMessageStream(3); } };
    services::WindowedMessageCommunication::WithReceiveBuffer<16> communication{ base };
    testing::StrictMock<MessageCommunicationObserverMock> observer{ communication };
};

TEST_F(WindowedMessageCommunicationTest, construction)
{
    FinishInitialization(4);
    EXPECT_CALL(base, MaxSendMessageSize()).WillOnce(testing::Return(20));
    EXPECT_EQ(19, communication.MaxSendMessageSize());
}

TEST_F(WindowedMessageCommunicationTest, send_message_after_initialized)
{
    FinishInitialization(6);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);
    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, message_waits_until_window_is_freed)
{
    FinishInitialization(2);

    communication.RequestSendMessage(4);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    SendReleaseWindow(4);

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, send_message_while_initializing_waits_for_initialized)
{
    communication.RequestSendMessage(4);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    FinishInitialization(6);

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, request_sending_new_message_while_previous_is_still_processing)
{
    FinishInitialization(12);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    communication.RequestSendMessage(2);

    ExpectSendMessageStream(3);
    ExpectSendMessageStreamAvailable({ 5, 6 });

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, receive_message_after_initialized)
{
    FinishInitialization(6);

    ReceivedMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    ExpectReceivedMessage(infra::ConstructBin()("abcd").Vector());
    ExpectSendReleaseWindow(6);
    ExecuteAllActions();
}

TEST_F(WindowedMessageCommunicationTest, received_message_before_initialized_is_discarded)
{
    ReceivedMessage(infra::ConstructBin().Value<uint8_t>(4)("abcd").Vector());

    FinishInitialization(6);

    ExecuteAllActions();
}

TEST_F(WindowedMessageCommunicationTest, received_release_window_before_initialized_is_discarded)
{
    communication.RequestSendMessage(4);
    SendReleaseWindow(6);

    OnSentData(infra::ConstructBin().Value<uint8_t>(1).Value<infra::LittleEndian<uint16_t>>(16).Vector());

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });
    SendInitResponse(6);
}

TEST_F(WindowedMessageCommunicationTest, handle_init_request_after_initialization)
{
    FinishInitialization(4);

    ExpectSendInitResponse(16);
    SendInitRequest(4);
    OnSentInitResponse(16);
}

TEST_F(WindowedMessageCommunicationTest, received_init_request_while_sending_message_finishes_message_then_sends_init_response)
{
    // build
    FinishInitialization(12);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    SendInitRequest(4);

    ExpectSendInitResponse(16);
    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
    OnSentInitResponse(16);
}

TEST_F(WindowedMessageCommunicationTest, increase_window_while_sending)
{
    // build
    FinishInitialization(6);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    SendReleaseWindow(4);

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    // Send a new message that uses the newly announced window
    ExpectSendMessageStream(3);
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, init_response_while_sending)
{
    // build
    FinishInitialization(6);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    communication.RequestSendMessage(4);

    // operate
    SendInitResponse(4);

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());

    // Send a new message that uses the newly announced window
    ExpectSendMessageStream(3);
    ExpectSendMessageStreamAvailable({ 5, 6 });
    communication.RequestSendMessage(2);
    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 5, 6 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, received_init_request_while_sending_init)
{
    SendInitRequest(4);

    ExpectSendInitResponse(16);
    FinishInitialization(12);

    OnSentInitResponse(16);
}

TEST_F(WindowedMessageCommunicationTest, init_response_while_sending_init_is_ignored)
{
    SendInitResponse(4);

    FinishInitialization(12);
}

TEST_F(WindowedMessageCommunicationTest, release_window_while_sending_init_is_ignored)
{
    SendReleaseWindow(4);

    FinishInitialization(12);
}

TEST_F(WindowedMessageCommunicationTest, requesting_message_while_sending_init_response)
{
    // build
    SendInitRequest(4);

    ExpectSendInitResponse(16);
    FinishInitialization(12);

    // operate
    communication.RequestSendMessage(4);

    ExpectSendMessageStream(5);
    ExpectSendMessageStreamAvailable({ 1, 2, 3, 4 });

    OnSentInitResponse(16);

    OnSentData(infra::ConstructBin().Value<uint8_t>(4)({ 1, 2, 3, 4 }).Vector());
}

TEST_F(WindowedMessageCommunicationTest, init_request_while_sending_init_response_results_in_new_init_response)
{
    // build
    SendInitRequest(4);

    ExpectSendInitResponse(16);
    FinishInitialization(12);

    // operate
    SendInitRequest(4);

    ExpectSendInitResponse(16);
    OnSentInitResponse(16);

    OnSentInitResponse(16);
}

TEST_F(WindowedMessageCommunicationTest, release_window_while_sending_init_response_is_ignored)
{
    // build
    SendInitRequest(4);

    ExpectSendInitResponse(16);
    FinishInitialization(12);

    // operate
    SendReleaseWindow(4);

    OnSentInitResponse(16);
}
