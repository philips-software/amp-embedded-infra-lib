#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "services/util/SesameInstantiation.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class SesameInstantiation
    {
    public:
        services::SerialCommunicationLoopback serial;

        constexpr static std::size_t leftSize = LeftSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<leftSize> leftSerial{ serial.Server() };
        main_::Sesame::WithMessageSize<leftSize> leftSesame{ leftSerial };
        testing::StrictMock<services::SesameObserverMock> leftUpper{ leftSesame.windowed };

        constexpr static std::size_t rightSize = RightSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<rightSize> rightSerial{ serial.Client() };
        main_::Sesame::WithMessageSize<rightSize> rightSesame{ rightSerial };
        testing::StrictMock<services::SesameObserverMock> rightUpper{ rightSesame.windowed };
    };
}

class SesameInstantiationTest
    : public testing::Test
    , public infra::ClockFixture
    , public SesameInstantiation<256, 1024>
{
public:
    SesameInstantiationTest()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }
};

TEST_F(SesameInstantiationTest, send_big_message_right)
{
    std::string sentData;

    EXPECT_CALL(leftUpper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([this, &sentData](infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            sentData = std::string(stream.Available(), 'a');
            stream << sentData;
        }));
    leftUpper.Subject().RequestSendMessage(leftUpper.Subject().MaxSendMessageSize());

    EXPECT_CALL(rightUpper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([this, &sentData](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            infra::TextInputStream::WithErrorPolicy stream(*reader);
            std::string text(stream.Available(), ' ');
            infra::BoundedString textString(text);
            stream >> textString;
            EXPECT_EQ(sentData, text);
        }));
    ExecuteAllActions();

    EXPECT_EQ(121, sentData.size());
}

TEST_F(SesameInstantiationTest, send_big_message_left)
{
    std::string sentData;

    EXPECT_CALL(rightUpper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([this, &sentData](infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            sentData = std::string(stream.Available(), 'a');
            stream << sentData;
        }));
    rightUpper.Subject().RequestSendMessage(rightUpper.Subject().MaxSendMessageSize());

    EXPECT_CALL(leftUpper, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([this, &sentData](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
        {
            infra::TextInputStream::WithErrorPolicy stream(*reader);
            std::string text(stream.Available(), ' ');
            infra::BoundedString textString(text);
            stream >> textString;
            EXPECT_EQ(sentData, text);
        }));
    ExecuteAllActions();

    EXPECT_EQ(121, sentData.size());
}

class SesameInstantiationTestMessageSize
    : public testing::TestWithParam<std::size_t>
    , public infra::ClockFixture
    , public SesameInstantiation<2048, 2048>
{
public:
    SesameInstantiationTestMessageSize()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }

    const std::size_t messageSize = GetParam();
};

TEST_P(SesameInstantiationTestMessageSize, send_message_of_size_right)
{
    EXPECT_EQ(1010, leftUpper.Subject().MaxSendMessageSize());

    std::string sentData1;
    std::string sentData2;

    EXPECT_CALL(leftUpper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([this, &sentData1](infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            sentData1 = std::string(messageSize, 'a');
            stream << sentData1;
        }));
    leftUpper.Subject().RequestSendMessage(messageSize);

    EXPECT_CALL(leftUpper, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([this, &sentData2](infra::SharedPtr<infra::StreamWriter>&& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(*writer);
            sentData2 = std::string(messageSize, 0);
            stream << sentData2;
        }));
    leftUpper.Subject().RequestSendMessage(messageSize);

    EXPECT_CALL(rightUpper, ReceivedMessage(testing::_))
        .WillOnce(testing::Invoke([this, &sentData1](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::TextInputStream::WithErrorPolicy stream(*reader);
                std::string text(stream.Available(), ' ');
                infra::BoundedString textString(text);
                stream >> textString;
                EXPECT_EQ(sentData1, text);
            }))
        .WillOnce(testing::Invoke([this, &sentData2](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
            {
                infra::TextInputStream::WithErrorPolicy stream(*reader);
                std::string text(stream.Available(), ' ');
                infra::BoundedString textString(text);
                stream >> textString;
                EXPECT_EQ(sentData2, text);
            }));
    ExecuteAllActions();

    EXPECT_EQ(messageSize, sentData1.size());
    EXPECT_EQ(messageSize, sentData2.size());
}

INSTANTIATE_TEST_SUITE_P(SesameInstantiationTestMessageSize, SesameInstantiationTestMessageSize, testing::Range<std::size_t>(1, 1011));
