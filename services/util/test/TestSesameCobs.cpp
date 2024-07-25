#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"
#include <deque>

class SesameCobsTest
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

    void ExpectSendSequence(std::vector<std::vector<uint8_t>> v)
    {
        auto first = v.front();
        auto last = v.back();
        v.erase(v.begin());
        v.pop_back();

        ExpectSendData(first);
        writer = nullptr;

        for (auto d : v)
        {
            ExpectSendData(d);
            onSent();
        }

        ExpectSendData(last);
        onSent();

        onSent();
    }

    void ExpectReceivedMessage(const std::vector<uint8_t>& expected, std::size_t encodedSize)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_, encodedSize)).WillOnce(testing::Invoke([this, expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, uint16_t encodedSize)
            {
                infra::DataInputStream::WithErrorPolicy stream(*reader);
                std::vector<uint8_t> data(stream.Available(), 0);
                stream >> infra::MakeRange(data);

                EXPECT_EQ(expected, data);
                EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(emptyReader)).RetiresOnSaturation();
            }));
    }

    void ExpectReceivedMessageKeepReader(const std::vector<uint8_t>& expected, std::size_t encodedSize)
    {
        EXPECT_CALL(observer, ReceivedMessage(testing::_, encodedSize)).WillOnce(testing::Invoke([this, expected](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, uint16_t encodedSize)
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
        EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(receivedDataReader)).RetiresOnSaturation();
        EXPECT_CALL(serial, AckReceived());
        serial.GetObserver().DataReceived();
    }

    testing::StrictMock<hal::BufferedSerialCommunicationMock> serial;
    services::SesameCobs::WithMaxMessageSize<280> communication{ serial };
    testing::StrictMock<services::SesameEncodedObserverMock> observer{ communication };
    infra::Function<void()> onSent;
    infra::StdVectorInputStreamReader::WithStorage receivedDataReader;
    infra::StdVectorInputStreamReader::WithStorage emptyReader;
    infra::SharedPtr<infra::StreamWriter> writer;
    infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
};

TEST_F(SesameCobsTest, MaxSendMessageSize)
{
    EXPECT_EQ(277, communication.MaxSendMessageSize());
}

TEST_F(SesameCobsTest, MessageSize)
{
    EXPECT_EQ(2, communication.MessageSize(0));
    EXPECT_EQ(255, communication.MessageSize(253));
    EXPECT_EQ(257, communication.MessageSize(254));
}

TEST_F(SesameCobsTest, send_data)
{
    RequestSendMessage(4, 7);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendSequence({ { 0 }, { 5 }, { 1, 2, 3, 4 }, { 0 } });
}

TEST_F(SesameCobsTest, send_data_with_0)
{
    RequestSendMessage(4, 7);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 0, 3, 4 }).Range();

    ExpectSendSequence({ { 0 }, { 2 }, { 1 }, { 3 }, { 3, 4 }, { 0 } });
}

TEST_F(SesameCobsTest, send_data_ending_with_0)
{
    RequestSendMessage(3, 6);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 5, 6, 0 }).Range();

    ExpectSendSequence({ { 0 }, { 3 }, { 5, 6 }, { 1 }, { 0 } });
}

TEST_F(SesameCobsTest, send_large_data)
{
    RequestSendMessage(280, 284);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()(std::vector<uint8_t>(280, 3)).Range();

    ExpectSendSequence({ { 0 }, { 255 }, std::vector<uint8_t>(254, 3), { 27 }, std::vector<uint8_t>(26, 3), { 0 } });
}

TEST_F(SesameCobsTest, send_two_packets)
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

TEST_F(SesameCobsTest, receive_data)
{
    ExpectReceivedMessage({ 1, 2, 3, 4 }, 7);
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 2, 3, 4, 0 }).Vector());
}

TEST_F(SesameCobsTest, receive_data_with_0)
{
    ExpectReceivedMessage({ 1, 0, 3, 4 }, 7);
    ReceiveData(infra::ConstructBin()({ 0, 2, 1, 3, 3, 4, 0 }).Vector());
}

TEST_F(SesameCobsTest, receive_large_data)
{
    ExpectReceivedMessage(std::vector<uint8_t>(280, 3), 284);
    ReceiveData(infra::ConstructBin()({ 0, 255 })(std::vector<uint8_t>(254, 3))({ 27 })(std::vector<uint8_t>(26, 3))({ 0 }).Vector());
}

TEST_F(SesameCobsTest, receive_interrupted_data)
{
    ExpectReceivedMessage({ 1, 2 }, 5);
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 2, 0 }).Vector());
}

TEST_F(SesameCobsTest, receive_two_messages)
{
    ExpectReceivedMessageKeepReader({ 1, 2, 3, 4 }, 7);
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 2, 3, 4, 0, 3, 5, 6, 0 }).Vector());

    ExpectReceivedMessage({ 5, 6 }, 4);
    EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(receivedDataReader)).RetiresOnSaturation();
    EXPECT_CALL(serial, AckReceived());
    reader = nullptr;
}

TEST_F(SesameCobsTest, malformed_empty_message_is_discarded)
{
    EXPECT_CALL(serial, Reader()).WillOnce(testing::ReturnRef(emptyReader)).RetiresOnSaturation();
    ReceiveData(infra::ConstructBin()({ 0, 5, 0 }).Vector());
}

TEST_F(SesameCobsTest, malformed_message_is_forwarded)
{
    ExpectReceivedMessage({ 1 }, 4);
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 0 }).Vector());
}

TEST_F(SesameCobsTest, no_new_received_message_after_stop)
{
    ExpectReceivedMessageKeepReader({ 1, 2, 3, 4 }, 7);
    ReceiveData(infra::ConstructBin()({ 0, 5, 1, 2, 3, 4, 0, 3, 5, 6, 0 }).Vector());

    communication.Stop();
    reader = nullptr;
}

TEST_F(SesameCobsTest, no_new_send_message_after_stop)
{
    EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
    communication.RequestSendMessage(4);

    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    communication.Stop();
    writer = nullptr;
}

TEST_F(SesameCobsTest, Reset_invalidates_RequestSendMessage)
{
    RequestSendMessage(4, 7);
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();
    }

    ExpectSendDataAndHandle({ 0 });
    ExpectSendDataAndHandle({ 5 });
    ExpectSendDataAndHandle({ 1, 2, 3, 4 });
    ExpectSendDataAndHandle({ 0 });
    writer = nullptr;

    EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
    communication.RequestSendMessage(4);

    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();
    }

    communication.Reset();
    writer = nullptr;

    RequestSendMessage(4, 7);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendSequence({ { 0 }, { 5 }, { 1, 2, 3, 4 }, { 0 } });
}

TEST_F(SesameCobsTest, Reset_after_first_delimiter_stops_sending_current_packet)
{
    EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
    communication.RequestSendMessage(4);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    communication.Reset();
    onSent();
}

TEST_F(SesameCobsTest, Reset_after_frame_stops_sending_current_packet)
{
    EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
    communication.RequestSendMessage(4);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    ExpectSendData({ 5 });
    onSent();

    communication.Reset();
    onSent();
}

TEST_F(SesameCobsTest, Reset_after_data_stops_sending_current_packet)
{
    EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
    communication.RequestSendMessage(4);
    infra::DataOutputStream::WithErrorPolicy stream(*writer);
    stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();

    ExpectSendData({ 0 });
    writer = nullptr;

    ExpectSendData({ 5 });
    onSent();

    ExpectSendData({ 1, 2, 3, 4 });
    onSent();

    communication.Reset();
    onSent();
}

TEST_F(SesameCobsTest, new_request_after_reset_is_handled)
{
    EXPECT_CALL(observer, SendMessageStreamAvailable).WillOnce(testing::SaveArg<0>(&writer));
    communication.RequestSendMessage(4);
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << infra::ConstructBin()({ 1, 2, 3, 4 }).Range();
    }

    ExpectSendData({ 0 });
    writer = nullptr;

    communication.Reset();

    RequestSendMessage(4, 5);
    onSent();

    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << infra::ConstructBin()({ 1, 2 }).Range();
    }

    ExpectSendSequence({ { 0 }, { 3 }, { 1, 2 }, { 0 } });
}
