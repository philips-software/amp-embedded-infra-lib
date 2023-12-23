#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/MessageCommunicationCobs.hpp"
#include "services/util/test_doubles/MessageCommunicationMock.hpp"
#include "gmock/gmock.h"
#include <deque>

class MessageCommunicationCobsTest
    : public testing::Test
{
public:
    void ExpectSendData(const std::vector<uint8_t>& v)
    {
        EXPECT_CALL(serial, SendData(infra::ContentsEqual(v), testing::_)).WillOnce(testing::SaveArg<1>(&onSent));
    }

    void ExpectSendDataAndHandle(const std::vector<uint8_t>& v)
    {
        EXPECT_CALL(serial, SendData(infra::ContentsEqual(v), testing::_)).WillOnce(testing::InvokeArgument<1>()).RetiresOnSaturation();
    }

    void ExpectReceivedMessage(const std::vector<uint8_t>& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([this, expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::DataInputStream::WithErrorPolicy stream(*reader);
                std::vector<uint8_t> data(stream.Available(), 0);
                stream >> infra::MakeRange(data);

                EXPECT_EQ(expected, data);
                EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(emptyReader)).RetiresOnSaturation();
            }));
    }

    void ExpectReceivedMessageKeepReader(const std::vector<uint8_t>& expected)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([this, expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::DataInputStream::WithErrorPolicy stream(*reader);
                std::vector<uint8_t> data(stream.Available(), 0);
                stream >> infra::MakeRange(data);

                EXPECT_EQ(expected, data);

                this->reader = std::move(reader);
            }));
    }

    void RequestSendMessage(uint16_t size, uint16_t encodedSize)
    {
        EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
        communication.RequestSendMessage(size);

        EXPECT_CALL(observer, MessageSent(encodedSize));
    }

    void ReceiveData(const std::vector<uint8_t>& data)
    {
        receivedDataReader.Storage() = data;
        EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(receivedDataReader)).WillOnce(testing::ReturnRef(emptyReader)).RetiresOnSaturation();
        EXPECT_CALL(serial, AckReceived());
        serial.GetObserver().DataReceived();
    }

    void ReceiveDataKeepReader(const std::vector<uint8_t>& data)
    {
        receivedDataReader.Storage() = data;
        EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(receivedDataReader)).RetiresOnSaturation();
        EXPECT_CALL(serial, AckReceived());
        serial.GetObserver().DataReceived();
    }

    testing::StrictMock<hal::BufferedSerialCommunicationMock> serial;
    services::MessageCommunicationCobs::WithMaxMessageSize<280> communication{ serial };
    testing::StrictMock<services::MessageCommunicationEncodedObserverMock> observer{ communication };
    infra::Function<void()> onSent;
    infra::StdVectorInputStreamReader::WithStorage receivedDataReader;
    infra::StdVectorInputStreamReader::WithStorage emptyReader;
    infra::SharedPtr<infra::StreamWriter> writer;
    infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
};

TEST_F(MessageCommunicationCobsTest, construction)
{
    EXPECT_EQ(280, communication.MaxSendMessageSize());
}

TEST_F(MessageCommunicationCobsTest, send_data)
{
    RequestSendMessage(4, 7);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    ExpectSendData({ 5 });
    onSent();

    ExpectSendData({ 1, 2, 3, 4 });
    onSent();

    ExpectSendData({ 0 });
    onSent();

    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_data_with_0)
{
    RequestSendMessage(4, 7);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 0, 3, 4 }).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    ExpectSendData({ 2 });
    onSent();

    ExpectSendData({ 1 }); // Data 1
    onSent();

    ExpectSendData({ 3 }); // Stuffing
    onSent();

    ExpectSendData({ 3, 4 }); // Data 3, 4
    onSent();

    ExpectSendData({ 0 });
    onSent();

    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_data_ending_with_0)
{
    RequestSendMessage(3, 6);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 5, 6, 0 }).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    ExpectSendData({ 3 });
    onSent();

    ExpectSendData({ 5, 6 }); // Data 1
    onSent();

    ExpectSendData({ 1 }); // Stuffing
    onSent();

    ExpectSendData({ 0 });
    onSent();

    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_large_data)
{
    RequestSendMessage(280, 284);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()(std::vector<uint8_t>(280, 3)).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    ExpectSendData({ 255 });
    onSent();

    ExpectSendData(std::vector<uint8_t>(254, 3));
    onSent();

    ExpectSendData({ 27 }); // Stuffing
    onSent();

    ExpectSendData(std::vector<uint8_t>(26, 3));
    onSent();

    ExpectSendData({ 0 });
    onSent();

    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_two_packets)
{
    RequestSendMessage(4, 7);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendDataAndHandle({ 0 });
    ExpectSendDataAndHandle({ 5 });
    ExpectSendDataAndHandle({ 1, 2, 3, 4 });
    ExpectSendDataAndHandle({ 0 });
    writer = nullptr;

    RequestSendMessage(2, 4);
    infra::DataOutputStream::WithErrorPolicy stream2(*writer);
    stream2 << infra::ConstructBin()({ 5, 6 }).Range();

    ExpectSendDataAndHandle({ 3 });
    ExpectSendDataAndHandle({ 5, 6 });
    ExpectSendDataAndHandle({ 0 });
    writer = nullptr;
}

TEST_F(MessageCommunicationCobsTest, receive_data)
{
    ExpectReceivedMessage({ 1, 2, 3, 4 });
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 2, 3, 4, 0 }).Vector());
}

TEST_F(MessageCommunicationCobsTest, receive_data_with_0)
{
    ExpectReceivedMessage({ 1, 0, 3, 4 });
    ReceiveData(infra::ConstructBin()({ 0, 2, 1, 3, 3, 4, 0 }).Vector());
}

TEST_F(MessageCommunicationCobsTest, receive_large_data)
{
    ExpectReceivedMessage(std::vector<uint8_t>(280, 3));
    ReceiveData(infra::ConstructBin()({ 0, 255 })(std::vector<uint8_t>(254, 3))({ 27 })(std::vector<uint8_t>(26, 3))({ 0 }).Vector());
}

TEST_F(MessageCommunicationCobsTest, receive_interrupted_data)
{
    ExpectReceivedMessage({ 1, 2 });
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 2, 0 }).Vector());
}

TEST_F(MessageCommunicationCobsTest, receive_two_messages)
{
    ExpectReceivedMessageKeepReader({ 1, 2, 3, 4 });
    ReceiveDataKeepReader(infra::ConstructBin()({ 0, 5, 1, 2, 3, 4, 0, 3, 5, 6, 0 }).Vector());

    ExpectReceivedMessage({ 5, 6 });
    EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(receivedDataReader)).WillOnce(testing::ReturnRef(emptyReader)).RetiresOnSaturation();
    EXPECT_CALL(serial, AckReceived());
    reader = nullptr;
}

TEST_F(MessageCommunicationCobsTest, malformed_empty_message_is_discarded)
{
    ReceiveData(infra::ConstructBin()({ 0, 5, 0 }).Vector());
}

TEST_F(MessageCommunicationCobsTest, malformed_message_is_forwarded)
{
    ExpectReceivedMessage({ 1 });
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 0 }).Vector());
}
