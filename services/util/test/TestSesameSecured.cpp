#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class AesGcmEncryptionMock
        : public services::AesGcmEncryption
    {
    public:
        MOCK_METHOD(void, SetEncryptKey, (infra::ConstByteRange key), (override));
        MOCK_METHOD(void, SetDecryptKey, (infra::ConstByteRange key), (override));
        MOCK_METHOD(void, Process, (infra::ConstByteRange iv, infra::ByteRange data, infra::ByteRange mac, const infra::Function<void()>& onDone), (override));
    };

    struct PendingOperation
    {
        std::vector<uint8_t> iv;
        infra::ByteRange data;
        infra::ByteRange mac;
        infra::Function<void()> onDone;
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

    void ExpectProcess(AesGcmEncryptionMock& encryption, PendingOperation& operation)
    {
        EXPECT_CALL(encryption, Process(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Invoke([&operation](infra::ConstByteRange iv, infra::ByteRange data, infra::ByteRange mac, const infra::Function<void()>& onDone)
            {
                operation = PendingOperation{ std::vector<uint8_t>(iv.begin(), iv.end()), data, mac, onDone };
            }));
    }

    void Complete(PendingOperation& operation, const std::array<uint8_t, 16>& computedMac)
    {
        infra::Copy(infra::MakeRange(computedMac), operation.mac);
        std::exchange(operation.onDone, nullptr)();
    }

    void RequestSend(infra::BoundedConstString message, std::vector<uint8_t>& data, infra::SharedOptional<infra::StdVectorOutputStreamWriter>& lowerWriter)
    {
        EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(100));
        EXPECT_CALL(lower, RequestSendMessage(16 + message.size()));
        upper.Subject().RequestSendMessage(message.size());

        ExpectWriteByUpper(message);
        lower.GetObserver().SendMessageStreamAvailable(lowerWriter.Emplace(data));
    }

    void Send(infra::BoundedConstString message)
    {
        ExpectProcess(sendEncryption, sendOperation);
        RequestSend(message, sentData, writer);
    }

    void ExpectWriteByUpper(infra::BoundedConstString message)
    {
        EXPECT_CALL(upper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([message](infra::SharedPtr<infra::StreamWriter>&& writer)
            {
                infra::TextOutputStream::WithErrorPolicy stream(*writer);
                stream << message;
            }));
    }

    void Receive(const std::vector<uint8_t>& message)
    {
        receivedData = message;
        ExpectProcess(receiveEncryption, receiveOperation);
        lower.GetObserver().ReceivedMessage(reader.Emplace(receivedData));
    }

    void ExpectReceivedMessage(infra::BoundedConstString expected)
    {
        EXPECT_CALL(upper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::TextInputStream::WithErrorPolicy stream(*reader);
                infra::BoundedString::WithStorage<64> s;
                s.resize(stream.Available());
                stream >> s;
                EXPECT_THAT(s, testing::Eq(expected));
            }));
    }

    std::vector<uint8_t> EncodedMessage(infra::BoundedConstString payload, const std::array<uint8_t, 16>& messageMac) const
    {
        std::vector<uint8_t> result(payload.begin(), payload.end());
        result.insert(result.end(), messageMac.begin(), messageMac.end());
        return result;
    }

    services::SesameSecured::KeyType key{ 1, 2 };
    services::SesameSecured::IvType iv{ 1, 3 };
    services::SesameSecured::IvType nextIv{ 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    services::SesameSecured::KeyType otherKey{ 4, 5 };
    services::SesameSecured::IvType otherIv{ 6, 7 };
    std::array<uint8_t, 16> mac{ 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf };
    std::array<uint8_t, 16> otherMac{};

    PendingOperation sendOperation;
    PendingOperation receiveOperation;
    std::vector<uint8_t> receivedData;
    infra::SharedOptional<infra::StdVectorInputStreamReader> reader;
    std::vector<uint8_t> sentData;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> writer;

    testing::StrictMock<services::SesameMock> lower;
    testing::StrictMock<AesGcmEncryptionMock> sendEncryption;
    testing::StrictMock<AesGcmEncryptionMock> receiveEncryption;
    infra::Execute expectInitialKeys{ [this]()
        {
            EXPECT_CALL(sendEncryption, SetEncryptKey(infra::ContentsEqual(key))).Times(2);
            EXPECT_CALL(receiveEncryption, SetDecryptKey(infra::ContentsEqual(key))).Times(2);
        } };
    services::SesameSecured::WithBuffers<64> secured{ lower, sendEncryption, receiveEncryption, services::SesameSecured::KeyMaterial{ key, iv, key, iv } };
    testing::StrictMock<services::SesameObserverMock> upper{ secured };
    testing::StrictMock<services::IntegrityObserverMock> integrityObserver{ secured };
};

TEST_F(SesameSecuredTest, message_is_written_to_lower_layer_only_after_encryption_completes)
{
    Send("abcd");

    ASSERT_THAT(sendOperation.iv, testing::ElementsAreArray(iv));
    ASSERT_THAT(sendOperation.data, testing::ElementsAre('a', 'b', 'c', 'd'));
    ASSERT_THAT(sentData, testing::IsEmpty());
    ASSERT_THAT(writer.Allocatable(), testing::IsFalse());

    Complete(sendOperation, mac);

    EXPECT_THAT(sentData, testing::ElementsAre('a', 'b', 'c', 'd', 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf));
    EXPECT_THAT(writer.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, next_message_is_encrypted_with_incremented_iv)
{
    Send("abcd");
    Complete(sendOperation, mac);
    sentData.clear();

    Send("efgh");

    EXPECT_THAT(sendOperation.iv, testing::ElementsAreArray(nextIv));
}

TEST_F(SesameSecuredTest, received_message_is_forwarded_only_after_decryption_completes_with_matching_mac)
{
    Receive(EncodedMessage("abcd", mac));

    ASSERT_THAT(receiveOperation.iv, testing::ElementsAreArray(iv));
    ASSERT_THAT(receiveOperation.data, testing::ElementsAre('a', 'b', 'c', 'd'));
    ASSERT_THAT(reader.Allocatable(), testing::IsFalse());

    ExpectReceivedMessage("abcd");
    Complete(receiveOperation, mac);

    EXPECT_THAT(reader.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, next_received_message_is_decrypted_with_incremented_iv)
{
    Receive(EncodedMessage("abcd", mac));
    ExpectReceivedMessage("abcd");
    Complete(receiveOperation, mac);

    Receive(EncodedMessage("efgh", mac));

    EXPECT_THAT(receiveOperation.iv, testing::ElementsAreArray(nextIv));
}

TEST_F(SesameSecuredTest, mismatching_mac_releases_message_and_is_reported_after_1_second)
{
    Receive(EncodedMessage("abcd", mac));
    Complete(receiveOperation, otherMac);

    ASSERT_THAT(reader.Allocatable(), testing::IsTrue());

    EXPECT_CALL(integrityObserver, IntegrityCheckFailed());
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(SesameSecuredTest, message_after_mismatching_mac_is_reported_immediately_without_decryption)
{
    Receive(EncodedMessage("abcd", mac));
    Complete(receiveOperation, otherMac);

    EXPECT_CALL(integrityObserver, IntegrityCheckFailed());
    std::vector<uint8_t> message = EncodedMessage("efgh", mac);
    lower.GetObserver().ReceivedMessage(reader.Emplace(message));
}

TEST_F(SesameSecuredTest, short_message_is_not_decrypted)
{
    std::vector<uint8_t> message{ 1, 2, 3 };
    lower.GetObserver().ReceivedMessage(reader.Emplace(message));

    EXPECT_THAT(reader.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, send_key_change_during_encryption_is_applied_after_completion)
{
    Send("abcd");
    secured.SetSendKey(otherKey, otherIv);

    EXPECT_CALL(sendEncryption, SetEncryptKey(infra::ContentsEqual(otherKey)));
    Complete(sendOperation, mac);
    sentData.clear();

    Send("efgh");

    EXPECT_THAT(sendOperation.iv, testing::ElementsAreArray(otherIv));
}

TEST_F(SesameSecuredTest, receive_key_change_during_decryption_is_applied_after_completion)
{
    Receive(EncodedMessage("abcd", mac));
    secured.SetReceiveKey(otherKey, otherIv);

    EXPECT_CALL(receiveEncryption, SetDecryptKey(infra::ContentsEqual(otherKey)));
    ExpectReceivedMessage("abcd");
    Complete(receiveOperation, mac);

    Receive(EncodedMessage("efgh", mac));

    EXPECT_THAT(receiveOperation.iv, testing::ElementsAreArray(otherIv));
}

TEST_F(SesameSecuredTest, initialization_during_encryption_discards_the_message_and_restores_initial_key_afterwards)
{
    Send("abcd");

    EXPECT_CALL(receiveEncryption, SetDecryptKey(infra::ContentsEqual(key)));
    EXPECT_CALL(upper, Initialized());
    lower.GetObserver().Initialized();

    EXPECT_CALL(sendEncryption, SetEncryptKey(infra::ContentsEqual(key)));
    Complete(sendOperation, mac);

    EXPECT_THAT(sentData, testing::IsEmpty());
    EXPECT_THAT(writer.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, send_stream_offered_during_discarded_encryption_is_handed_to_upper_layer_after_completion)
{
    Send("abcd");

    EXPECT_CALL(receiveEncryption, SetDecryptKey(infra::ContentsEqual(key)));
    EXPECT_CALL(upper, Initialized());
    lower.GetObserver().Initialized();

    std::vector<uint8_t> secondData;
    infra::SharedOptional<infra::StdVectorOutputStreamWriter> secondWriter;
    EXPECT_CALL(lower, MaxSendMessageSize()).WillOnce(testing::Return(100));
    EXPECT_CALL(lower, RequestSendMessage(16 + 4));
    upper.Subject().RequestSendMessage(4);
    lower.GetObserver().SendMessageStreamAvailable(secondWriter.Emplace(secondData));

    PendingOperation secondOperation;
    EXPECT_CALL(sendEncryption, SetEncryptKey(infra::ContentsEqual(key)));
    ExpectWriteByUpper("efgh");
    ExpectProcess(sendEncryption, secondOperation);
    Complete(sendOperation, mac);

    ASSERT_THAT(secondOperation.iv, testing::ElementsAreArray(iv));
    Complete(secondOperation, mac);

    EXPECT_THAT(sentData, testing::IsEmpty());
    EXPECT_THAT(secondData, testing::ElementsAre('e', 'f', 'g', 'h', 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf));
}

TEST_F(SesameSecuredTest, reset_during_decryption_releases_the_message_and_ignores_the_result)
{
    Receive(EncodedMessage("abcd", mac));

    EXPECT_CALL(lower, Reset());
    upper.Subject().Reset();

    ASSERT_THAT(reader.Allocatable(), testing::IsTrue());

    Complete(receiveOperation, mac);
}

TEST_F(SesameSecuredTest, message_received_during_discarded_decryption_is_decrypted_after_completion)
{
    Receive(EncodedMessage("abcd", mac));

    EXPECT_CALL(lower, Reset());
    upper.Subject().Reset();

    std::vector<uint8_t> secondMessage = EncodedMessage("efgh", mac);
    infra::SharedOptional<infra::StdVectorInputStreamReader> secondReader;
    lower.GetObserver().ReceivedMessage(secondReader.Emplace(secondMessage));

    PendingOperation secondOperation;
    ExpectProcess(receiveEncryption, secondOperation);
    Complete(receiveOperation, mac);

    ASSERT_THAT(secondOperation.data, testing::ElementsAre('e', 'f', 'g', 'h'));

    ExpectReceivedMessage("efgh");
    Complete(secondOperation, mac);

    EXPECT_THAT(secondReader.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, reset_reading_during_decryption_ignores_the_result)
{
    Receive(EncodedMessage("abcd", mac));

    EXPECT_CALL(lower, ResetReading());
    upper.Subject().ResetReading();

    Complete(receiveOperation, mac);

    EXPECT_THAT(reader.Allocatable(), testing::IsTrue());
}

TEST_F(SesameSecuredTest, reset_is_forwarded)
{
    EXPECT_CALL(lower, Reset());
    upper.Subject().Reset();
}
