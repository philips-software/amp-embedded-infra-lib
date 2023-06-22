#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ConstructBin.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/MessageCommunicationCobs.hpp"
#include "gmock/gmock.h"
#include <deque>

namespace
{
    class MessageCommunicationReceiveOnInterruptObserverMock
        : public services::MessageCommunicationReceiveOnInterruptObserver
    {
    public:
        using services::MessageCommunicationReceiveOnInterruptObserver::MessageCommunicationReceiveOnInterruptObserver;

        MOCK_METHOD1(ReceivedMessageOnInterrupt, void(infra::StreamReader& reader));
    };
}

class MessageCommunicationCobsTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    void ExpectSendData(const std::vector<uint8_t>& v)
    {
        EXPECT_CALL(serial, SendData(testing::_, testing::_)).WillOnce(testing::Invoke([this, v](infra::ConstByteRange data, infra::Function<void()> onDone)
            {
                EXPECT_EQ(v, data);
                onSent = onDone;
            }));
    }

    void ExpectReceivedMessage(const std::vector<uint8_t>& expected)
    {
        EXPECT_CALL(observer, ReceivedMessageOnInterrupt(testing::_)).WillOnce(testing::Invoke([this, expected](infra::StreamReader& reader)
            {
                infra::DataInputStream::WithErrorPolicy stream(reader);
                std::vector<uint8_t> data(stream.Available(), 0);
                stream >> infra::MakeRange(data);

                EXPECT_EQ(expected, data);
            }));
    }

    testing::StrictMock<hal::SerialCommunicationCleanMock> serial;
    infra::Function<void(infra::ConstByteRange data)> receiveData;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(serial, ReceiveData(testing::_)).WillOnce(testing::SaveArg<0>(&receiveData));
        } };
    services::MessageCommunicationCobs::WithMaxMessageSize<280> communication{ serial };
    testing::StrictMock<MessageCommunicationReceiveOnInterruptObserverMock> observer{ communication };
    infra::Function<void()> onSent;
};

TEST_F(MessageCommunicationCobsTest, construction)
{
    EXPECT_EQ(280, communication.MaxSendMessageSize());
}

TEST_F(MessageCommunicationCobsTest, send_data)
{
    testing::StrictMock<infra::MockCallback<void(uint16_t size)>> callback;
    auto writer = communication.SendMessageStream(4, [&](uint16_t size)
        {
            callback.callback(size);
        });
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

    EXPECT_CALL(callback, callback(4));
    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_data_with_0)
{
    testing::StrictMock<infra::MockCallback<void(uint16_t size)>> callback;
    auto writer = communication.SendMessageStream(4, [&](uint16_t size)
        {
            callback.callback(size);
        });
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

    EXPECT_CALL(callback, callback(4));
    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_data_ending_with_0)
{
    testing::StrictMock<infra::MockCallback<void(uint16_t size)>> callback;
    auto writer = communication.SendMessageStream(3, [&](uint16_t size)
        {
            callback.callback(size);
        });
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

    EXPECT_CALL(callback, callback(3));
    onSent();
}

TEST_F(MessageCommunicationCobsTest, send_large_data)
{
    testing::StrictMock<infra::MockCallback<void(uint16_t size)>> callback;
    auto writer = communication.SendMessageStream(280, [&](uint16_t size)
        {
            callback.callback(size);
        });
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

    EXPECT_CALL(callback, callback(280));
    onSent();
}

TEST_F(MessageCommunicationCobsTest, receive_data)
{
    ExpectReceivedMessage({ 1, 2, 3, 4 });
    receiveData(infra::ConstructBin()({ 0, 5, 1, 2, 3, 4, 0 }).Range());
}

TEST_F(MessageCommunicationCobsTest, receive_data_with_0)
{
    ExpectReceivedMessage({ 1, 0, 3, 4 });
    receiveData(infra::ConstructBin()({ 0, 2, 1, 3, 3, 4, 0 }).Range());
}

TEST_F(MessageCommunicationCobsTest, receive_large_data)
{
    ExpectReceivedMessage(std::vector<uint8_t>(280, 3));
    receiveData(infra::ConstructBin()({ 0, 255 })(std::vector<uint8_t>(254, 3))({ 27 })(std::vector<uint8_t>(26, 3))({ 0 }).Range());
}

TEST_F(MessageCommunicationCobsTest, receive_interrupted_data)
{
    ExpectReceivedMessage({ 1, 2 });
    receiveData(infra::ConstructBin()({ 0, 5, 1, 2, 0 }).Range());
}
