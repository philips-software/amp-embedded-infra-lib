#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SesameCryptoMbedTls.hpp"
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
        MOCK_METHOD(void, Encrypt, (infra::ConstByteRange iv, infra::ByteRange data, infra::ByteRange mac), (override));
        MOCK_METHOD(bool, Decrypt, (infra::ConstByteRange iv, infra::ByteRange data, infra::ConstByteRange mac), (override));
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

    void ReleaseSendStreamWithoutPayload(std::size_t requestedSize)
    {
        EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(100));
        EXPECT_CALL(lower, RequestSendMessage(16 + requestedSize));
        upper.Subject().RequestSendMessage(requestedSize);

        EXPECT_CALL(upper, SendMessageStreamAvailable(testing::_));
        SendMessageStreamAvailable(sentData);
    }

    services::SesameSecured::KeyType key{ 1, 2 };
    services::SesameSecured::IvType iv{ 1, 3 };

    testing::StrictMock<services::SesameMock> lower;
    services::SesameSecuredMbedTlsEncryptors encryptors;
    services::SesameSecured::WithMessageSize<64> secured{ encryptors, lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
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

TEST_F(SesameSecuredTest, max_send_message_size_is_limited_by_owned_storage)
{
    EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(100));

    EXPECT_THAT(secured.MaxSendMessageSize(), testing::Eq(64));
}

TEST_F(SesameSecuredTest, max_send_message_size_is_limited_by_delegate)
{
    EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(48));

    EXPECT_THAT(secured.MaxSendMessageSize(), testing::Eq(32));
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
    for (auto i = 1; i != 34; ++i)
    {
        infra::BoundedConstString message("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        message.shrink(i);

        Send(message);
        Receive(message);
    }
}

TEST_F(SesameSecuredTest, send_stream_released_without_payload_sends_nothing)
{
    ReleaseSendStreamWithoutPayload(4);

    EXPECT_THAT(sentData, testing::IsEmpty());
    EXPECT_THAT(writer.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, message_after_send_stream_released_without_payload_is_received)
{
    ReleaseSendStreamWithoutPayload(4);

    Send("abcd");
    Receive("abcd");
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

TEST_F(SesameSecuredStandaloneTest, message_size_wrapper_initializes_keys_from_symmetric_key_file)
{
    sesame_security::SymmetricKeyFile keyMaterial;
    keyMaterial.sendBySelf.key.resize(services::SesameSecured::keySize);
    keyMaterial.sendBySelf.iv.resize(services::SesameSecured::ivSize);
    keyMaterial.sendByOther.key.resize(services::SesameSecured::keySize);
    keyMaterial.sendByOther.iv.resize(services::SesameSecured::ivSize);
    keyMaterial.sendBySelf.key[0] = 1;
    keyMaterial.sendByOther.key[0] = 2;
    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    services::AesGcmEncryptors encryptors{ sendEncryption, receiveEncryption };

    EXPECT_CALL(sendEncryption, EncryptWithKey(testing::ElementsAreArray(keyMaterial.sendBySelf.key)));
    EXPECT_CALL(receiveEncryption, DecryptWithKey(testing::ElementsAreArray(keyMaterial.sendByOther.key)));

    services::SesameSecured::WithMessageSize<64> secured{ encryptors, lower, keyMaterial };
}

TEST_F(SesameSecuredStandaloneTest, received_message_longer_than_one_block_is_decrypted_in_one_call)
{
    services::SesameSecured::KeyType key{};
    services::SesameSecured::IvType iv{};
    std::array<uint8_t, 20> encryptedPayload{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T' };
    std::array<uint8_t, 20> decryptedPayload{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't' };
    std::array<uint8_t, services::SesameSecured::blockSize> computedMac{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> sendBuffer;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> receiveBuffer;

    EXPECT_CALL(sendEncryption, EncryptWithKey(testing::_));
    EXPECT_CALL(receiveEncryption, DecryptWithKey(testing::_));
    services::AesGcmEncryptors encryptors{ sendEncryption, receiveEncryption };
    services::SesameSecured::StorageBase storage{ sendBuffer, receiveBuffer };
    services::SesameSecured secured(storage, encryptors, lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv });
    testing::StrictMock<services::SesameObserverMock> upper{ secured };

    EXPECT_CALL(receiveEncryption, Decrypt(testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([&](infra::ConstByteRange, infra::ByteRange data, infra::ConstByteRange mac)
        {
            EXPECT_THAT(std::vector<uint8_t>(data.begin(), data.end()), testing::ElementsAreArray(encryptedPayload));
            EXPECT_THAT(std::vector<uint8_t>(mac.begin(), mac.end()), testing::ElementsAreArray(computedMac));
            infra::Copy(infra::MakeRange(decryptedPayload), data);
            return true;
        }));

    EXPECT_CALL(upper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            infra::TextInputStream::WithErrorPolicy stream(*reader);
            EXPECT_THAT(stream.Available(), testing::Eq(20));
            infra::BoundedString::WithStorage<32> s;
            s.resize(stream.Available());
            stream >> s;
            EXPECT_THAT(s, testing::Eq("abcdefghijklmnopqrst"));
        }));

    std::vector<uint8_t> encodedMessage(encryptedPayload.begin(), encryptedPayload.end());
    encodedMessage.insert(encodedMessage.end(), computedMac.begin(), computedMac.end());
    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    lower.GetObserver().ReceivedMessage(reader.Emplace(encodedMessage));
}

TEST_F(SesameSecuredStandaloneTest, received_message_with_mac_mismatch_is_reported_and_not_delivered)
{
    services::SesameSecured::KeyType key{};
    services::SesameSecured::IvType iv{};
    std::array<uint8_t, 5> encryptedPayload{ 'a', 'b', 'q', 'r', 's' };
    std::array<uint8_t, services::SesameSecured::blockSize> receivedMac{};

    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> sendBuffer;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> receiveBuffer;

    EXPECT_CALL(sendEncryption, EncryptWithKey(testing::_));
    EXPECT_CALL(receiveEncryption, DecryptWithKey(testing::_));
    services::AesGcmEncryptors encryptors{ sendEncryption, receiveEncryption };
    services::SesameSecured::StorageBase storage{ sendBuffer, receiveBuffer };
    services::SesameSecured secured(storage, encryptors, lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv });
    testing::StrictMock<services::SesameObserverMock> upper{ secured };
    testing::StrictMock<services::IntegrityObserverMock> integrityObserver{ secured };

    EXPECT_CALL(receiveEncryption, Decrypt(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(false));

    std::vector<uint8_t> encodedMessage(encryptedPayload.begin(), encryptedPayload.end());
    encodedMessage.insert(encodedMessage.end(), receivedMac.begin(), receivedMac.end());
    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    lower.GetObserver().ReceivedMessage(reader.Emplace(encodedMessage));

    EXPECT_CALL(integrityObserver, IntegrityCheckFailed());
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(SesameSecuredStandaloneTest, received_message_without_payload_is_decrypted_with_empty_data)
{
    services::SesameSecured::KeyType key{};
    services::SesameSecured::IvType iv{};
    std::array<uint8_t, services::SesameSecured::blockSize> receivedMac{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> sendBuffer;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> receiveBuffer;

    EXPECT_CALL(sendEncryption, EncryptWithKey(testing::_));
    EXPECT_CALL(receiveEncryption, DecryptWithKey(testing::_));
    services::AesGcmEncryptors encryptors{ sendEncryption, receiveEncryption };
    services::SesameSecured::StorageBase storage{ sendBuffer, receiveBuffer };
    services::SesameSecured secured(storage, encryptors, lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv });
    testing::StrictMock<services::SesameObserverMock> upper{ secured };

    EXPECT_CALL(receiveEncryption, Decrypt(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Invoke([&](infra::ConstByteRange, infra::ByteRange data, infra::ConstByteRange mac)
        {
            EXPECT_THAT(data.size(), testing::Eq(0));
            EXPECT_THAT(std::vector<uint8_t>(mac.begin(), mac.end()), testing::ElementsAreArray(receivedMac));
            return true;
        }));
    EXPECT_CALL(upper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            infra::TextInputStream::WithErrorPolicy stream(*reader);
            EXPECT_THAT(stream.Available(), testing::Eq(0));
        }));

    std::vector<uint8_t> encodedMessage(receivedMac.begin(), receivedMac.end());
    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    lower.GetObserver().ReceivedMessage(reader.Emplace(encodedMessage));
}

TEST_F(SesameSecuredStandaloneTest, sent_message_is_encrypted_in_one_call_with_mac_in_tail)
{
    services::SesameSecured::KeyType key{};
    services::SesameSecured::IvType iv{ 1, 2, 3 };
    const std::string plain = "abcdefghijklmnopqrstu";
    const std::vector<uint8_t> encrypted{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U' };
    std::array<uint8_t, services::SesameSecured::blockSize> mac{ 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> sendBuffer;
    infra::BoundedVector<uint8_t>::WithMaxSize<64> receiveBuffer;

    EXPECT_CALL(sendEncryption, EncryptWithKey(testing::_));
    EXPECT_CALL(receiveEncryption, DecryptWithKey(testing::_));
    services::AesGcmEncryptors encryptors{ sendEncryption, receiveEncryption };
    services::SesameSecured::StorageBase storage{ sendBuffer, receiveBuffer };
    services::SesameSecured secured(storage, encryptors, lower, services::SesameSecured::KeyMaterial{ key, iv, key, iv });
    testing::StrictMock<services::SesameObserverMock> upper{ secured };

    EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(100));
    EXPECT_CALL(lower, RequestSendMessage(plain.size() + services::SesameSecured::blockSize));
    upper.Subject().RequestSendMessage(plain.size());

    EXPECT_CALL(upper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([&](infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            stream << infra::BoundedConstString(plain.data(), plain.size());
        }));
    std::vector<uint8_t> sentData;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;

    EXPECT_CALL(sendEncryption, Encrypt(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Invoke([&](infra::ConstByteRange ivUsed, infra::ByteRange data, infra::ByteRange macOut)
        {
            EXPECT_THAT(std::vector<uint8_t>(ivUsed.begin(), ivUsed.end()), testing::ElementsAreArray(iv));
            EXPECT_THAT(std::string(data.begin(), data.end()), testing::Eq(plain));
            EXPECT_THAT(macOut.size(), testing::Eq(services::SesameSecured::blockSize));
            infra::Copy(infra::MakeRange(encrypted), data);
            infra::Copy(infra::MakeRange(mac), macOut);
        }));
    lower.GetObserver().SendMessageStreamAvailable(writer.Emplace(sentData));

    std::vector<uint8_t> expected(encrypted);
    expected.insert(expected.end(), mac.begin(), mac.end());
    EXPECT_THAT(sentData, testing::ElementsAreArray(expected));
}
