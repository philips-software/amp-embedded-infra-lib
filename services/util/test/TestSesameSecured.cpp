#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class AesGcmEncryptionMock
        : public services::AesGcmEncryption
    {
    public:
        MOCK_METHOD(void, EncryptWithKey, (infra::ConstByteRange key), (override));
        MOCK_METHOD(void, DecryptWithKey, (infra::ConstByteRange key), (override));
        MOCK_METHOD(void, Start, (infra::ConstByteRange iv), (override));
        MOCK_METHOD(std::size_t, Update, (infra::ConstByteRange from, infra::ByteRange to), (override));
        MOCK_METHOD(std::size_t, Finish, (infra::ByteRange to, infra::ByteRange mac), (override));
    };
}

class SesameSecuredTest
    : public testing::Test
    , public infra::ClockFixture
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

    services::SesameSecured::KeyType key{ 1, 2 };
    services::SesameSecured::IvType iv{ 1, 3 };

    testing::StrictMock<services::SesameMock> lower;
    services::SesameSecured::WithCryptoMbedTls::WithBuffers<64> secured{ lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    testing::StrictMock<services::SesameObserverMock> upper{ secured };
    testing::StrictMock<services::IntegrityObserverMock> integrityObserver{ secured };

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

    services::SesameSecured::KeyType key2{ 1, 2, 1 };
    services::SesameSecured::IvType iv2{ 1, 3, 1 };
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

    services::SesameSecured::KeyType key2{ 1, 2, 1 };
    services::SesameSecured::IvType iv2{ 1, 3, 1 };
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

    Send("efgh");
    EXPECT_CALL(integrityObserver, IntegrityCheckFailed());
    ReceivedMessage(sentData);
}

TEST_F(SesameSecuredTest, damaged_message_is_reported_after_1_second)
{
    Send("abcd");

    sentData[7] = 7;

    ReceivedMessage(sentData);

    EXPECT_CALL(integrityObserver, IntegrityCheckFailed());
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(SesameSecuredTest, truncated_message_followed_by_init_is_not_reported)
{
    Send("abcd");

    sentData.resize(sentData.size() - 1);

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).Times(0);
    ReceivedMessage(sentData);

    EXPECT_CALL(upper, Initialized());
    lower.GetObserver().Initialized();
    sentData.clear();

    Send("efgh");
    Receive("efgh");
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

class SesameSecuredStandaloneTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(SesameSecuredStandaloneTest, received_message_includes_non_zero_finish_output)
{
    services::SesameSecured::KeyType key{};
    services::SesameSecured::IvType iv{};
    std::array<uint8_t, 2> updateInput{ 'a', 'b' };
    std::array<uint8_t, 3> finishOutput{ 'x', 'y', 'z' };
    std::array<uint8_t, services::SesameSecured::blockSize> computedMac{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> sendBuffer;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> receiveBuffer;

    EXPECT_CALL(sendEncryption, EncryptWithKey(testing::_));
    EXPECT_CALL(receiveEncryption, DecryptWithKey(testing::_));
    services::SesameSecured secured(sendEncryption, receiveEncryption, sendBuffer, receiveBuffer, lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv });
    testing::StrictMock<services::SesameObserverMock> upper{ secured };

    EXPECT_CALL(receiveEncryption, Start(testing::_));
    EXPECT_CALL(receiveEncryption, Update(testing::_, testing::_)).WillOnce(testing::Invoke([](infra::ConstByteRange from, infra::ByteRange to)
        {
            infra::Copy(from, infra::Head(to, from.size()));
            return from.size();
        }));
    EXPECT_CALL(receiveEncryption, Finish(testing::_, testing::_)).WillOnce(testing::Invoke([&](infra::ByteRange to, infra::ByteRange mac)
        {
            EXPECT_GE(to.size(), finishOutput.size());
            EXPECT_EQ(computedMac.size(), mac.size());
            infra::Copy(infra::MakeRange(finishOutput), infra::Head(to, std::min(to.size(), finishOutput.size())));
            infra::Copy(infra::MakeRange(computedMac), mac);
            return finishOutput.size();
        }));

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            infra::TextInputStream::WithErrorPolicy stream(*reader);
            EXPECT_EQ(5, stream.Available());
            infra::BoundedString::WithStorage<8> s;
            s.resize(stream.Available());
            stream >> s;
            EXPECT_EQ("abxyz", s);
        }));

    std::vector<uint8_t> encodedMessage(updateInput.begin(), updateInput.end());
    encodedMessage.insert(encodedMessage.end(), computedMac.begin(), computedMac.end());
    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    lower.GetObserver().ReceivedMessage(reader.Emplace(encodedMessage));
}
