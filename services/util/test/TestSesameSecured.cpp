#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

class SesameSecuredTest
    : public testing::Test
{
public:
    SesameSecuredTest()
    {
        EXPECT_CALL(upper, Initialized());
        lower.GetObserver().Initialized();
    }

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
                infra::BoundedString::WithStorage<256> s;
                s.resize(stream.Available());
                stream >> s;
                EXPECT_EQ(expected, s);
            }));
    }

    void ExpectSendMessageStreamAvailable(infra::BoundedConstString message)
    {
        EXPECT_CALL(upper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([message](infra::SharedPtr<infra::StreamWriter>&& writer)
            {
                infra::TextOutputStream::WithErrorPolicy stream(*writer);
                EXPECT_EQ(message.size(), stream.Available());
                stream << message;
            }));
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

    std::array<uint8_t, services::SesameSecured::keySize> key{ 1, 2 };
    std::array<uint8_t, services::SesameSecured::blockSize> iv{ 1, 3 };

    testing::StrictMock<services::SesameMock> lower;
    services::SesameSecured::WithCryptoMbedTls::WithBuffers<64> secured{ lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    testing::StrictMock<services::SesameObserverMock> upper{ secured };

    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    std::vector<uint8_t> sentData;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;
};

TEST_F(SesameSecuredTest, send_receive_message)
{
    Send("abcd");
    Receive("abcd");
}

TEST_F(SesameSecuredTest, send_receive_two_messages)
{
    Send("abcd");
    Receive("abcd");

    Send("efghi");
    Receive("efghi");
}

TEST_F(SesameSecuredTest, same_consecutive_messages_are_encrypted_differently)
{
    Send("abcd");
    auto first = sentData;
    Receive("abcd");

    Send("abcd");
    auto second = sentData;

    EXPECT_NE(first, second);
}

TEST_F(SesameSecuredTest, key_change_to_default_key_results_in_same_encryption)
{
    Send("abcd");
    auto first = sentData;
    Receive("abcd");

    secured.SetSendKey(key, iv);
    secured.SetReceiveKey(key, iv);

    Send("abcd");
    auto second = sentData;
    Receive("abcd");

    EXPECT_EQ(first, second);
}

TEST_F(SesameSecuredTest, key_change_to_different_key_results_in_different_encryption)
{
    Send("abcd");
    auto first = sentData;
    Receive("abcd");

    std::array<uint8_t, services::SesameSecured::keySize> key2{ 1, 2, 1 };
    std::array<uint8_t, services::SesameSecured::blockSize> iv2{ 1, 3, 1 };
    secured.SetSendKey(key2, iv2);

    Send("abcd");
    auto second = sentData;

    EXPECT_NE(first, second);
}

TEST_F(SesameSecuredTest, initialization_results_in_default_keys)
{
    Send("abcd");
    auto first = sentData;
    Receive("abcd");

    std::array<uint8_t, services::SesameSecured::keySize> key2{ 1, 2, 1 };
    std::array<uint8_t, services::SesameSecured::blockSize> iv2{ 1, 3, 1 };
    secured.SetSendKey(key2, iv2);

    EXPECT_CALL(upper, Initialized());
    lower.GetObserver().Initialized();

    Send("abcd");
    auto second = sentData;
    Receive("abcd");

    EXPECT_EQ(first, second);
}

TEST_F(SesameSecuredTest, damaged_message_does_not_propagate)
{
    Send("abcd");

    sentData[7] = 7;

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).Times(0);
    ReceivedMessage(sentData);
}

TEST_F(SesameSecuredTest, short_message_does_not_propagate)
{
    Send("abcd");

    sentData.resize(3);

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).Times(0);
    ReceivedMessage(sentData);
}

TEST_F(SesameSecuredTest, different_sizes)
{
    for (auto i = 0; i != 34; ++i)
    {
        infra::BoundedConstString message("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        message.shrink(i);

        Send(message);
        Receive(message);
    }
}

TEST_F(SesameSecuredTest, Reset_is_forwarded)
{
    EXPECT_CALL(lower, Reset());
    upper.Subject().Reset();
}
