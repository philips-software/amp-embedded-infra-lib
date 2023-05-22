#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "services/util/MessageCommunicationSecured.hpp"
#include "services/util/test_doubles/MessageCommunicationMock.hpp"
#include "gmock/gmock.h"

class MessageCommunicationSecuredTest
    : public testing::Test
{
public:
    void ReceivedMessage(const std::vector<uint8_t>& message)
    {
        lower.GetObserver().ReceivedMessage(reader.Emplace(message));
    }

    void SendMessageStreamAvailable(std::vector<uint8_t>& message)
    {
        lower.GetObserver().SendMessageStreamAvailable(writer.Emplace(message));
    }

    void ExpectReceivedMessage(infra::BoundedConstString expected)
    {
        EXPECT_CALL(upper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([expected](infra::SharedPtr<infra::StreamReader>&& reader)
            {
            infra::TextInputStream::WithErrorPolicy stream(*reader);
            EXPECT_EQ(expected.size(), stream.Available());
            std::string s;
            s.resize(stream.Available());
            stream >> infra::BoundedString(s);
            EXPECT_EQ(expected, s); }));
    }

    void ExpectSendMessageStreamAvailable(infra::BoundedConstString message)
    {
        EXPECT_CALL(upper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([message](infra::SharedPtr<infra::StreamWriter>&& writer)
            {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            EXPECT_EQ(message.size(), stream.Available());
            stream << message; }));
    }

    void Send(infra::BoundedConstString message)
    {
        EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(100));
        EXPECT_CALL(lower, RequestSendMessage(16 + message.size()));

        upper.Subject().RequestSendMessage(message.size());
        ExpectSendMessageStreamAvailable(message);
        SendMessageStreamAvailable(sentData);
        EXPECT_TRUE(writer.Allocatable());
    }

    void Receive(infra::BoundedConstString message)
    {
        ExpectReceivedMessage(message);
        ReceivedMessage(sentData);
        sentData.clear();
    }

    std::array<uint8_t, services::MessageCommunicationSecured::keySize> key{ 1, 2 };
    std::array<uint8_t, services::MessageCommunicationSecured::blockSize> iv{ 1, 3 };

    testing::StrictMock<services::MessageCommunicationMock> lower;
    services::MessageCommunicationSecured::WithBuffers<32> secured{ lower, key, iv, key, iv };
    testing::StrictMock<services::MessageCommunicationObserverMock> upper{ secured };

    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    std::vector<uint8_t> sentData;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
};

TEST_F(MessageCommunicationSecuredTest, send_receive_message)
{
    Send("abcd");
    Receive("abcd");
}

TEST_F(MessageCommunicationSecuredTest, send_receive_two_messages)
{
    Send("abcd");
    Receive("abcd");

    Send("efghi");
    Receive("efghi");
}

TEST_F(MessageCommunicationSecuredTest, same_consecutive_messages_are_encrypted_differently)
{
    Send("abcd");
    auto first = sentData;
    Receive("abcd");

    Send("abcd");
    auto second = sentData;

    EXPECT_NE(first, second);
}

TEST_F(MessageCommunicationSecuredTest, damaged_message_does_not_propagate)
{
    Send("abcd");

    sentData[7] = 7;

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).Times(0);
    ReceivedMessage(sentData);
}

TEST_F(MessageCommunicationSecuredTest, short_message_does_not_propagate)
{
    Send("abcd");

    sentData.resize(3);

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).Times(0);
    ReceivedMessage(sentData);
}

TEST_F(MessageCommunicationSecuredTest, different_sizes)
{
    for (auto i = 0; i != 33; ++i)
    {
        infra::BoundedConstString message("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        message.shrink(i);

        Send(message);
        Receive(message);
    }
}
