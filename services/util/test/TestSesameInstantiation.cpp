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
    class SesameInstantiationPair
    {
    public:
        void ExpectSend(testing::StrictMock<services::SesameObserverMock>& sender, std::string& sentData, std::size_t messageSize, char fill)
        {
            EXPECT_CALL(sender, SendMessageStreamAvailable(testing::_)).WillOnce(testing::Invoke([&sentData, messageSize, fill](infra::SharedPtr<infra::StreamWriter>&& writer)
                {
                    infra::TextOutputStream::WithErrorPolicy stream(*writer);
                    sentData = std::string(messageSize, fill);
                    stream << sentData;
                }));
        }

        void ExpectReceive(testing::StrictMock<services::SesameObserverMock>& receiver, const std::string& sentData)
        {
            EXPECT_CALL(receiver, ReceivedMessage(testing::_)).WillOnce(testing::Invoke([&sentData](infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
                {
                    infra::TextInputStream::WithErrorPolicy stream(*reader);
                    std::string text(stream.Available(), ' ');
                    infra::BoundedString textString(text);
                    stream >> textString;
                    EXPECT_THAT(text, testing::Eq(sentData));
                }));
        }

        services::SerialCommunicationLoopback serial;

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<LeftSize> leftSerial{ serial.Server() };
        main_::Sesame::WithMessageSize<LeftSize> leftSesame{ leftSerial };
        testing::StrictMock<services::SesameObserverMock> leftUpper{ leftSesame.windowed };

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<RightSize> rightSerial{ serial.Client() };
        main_::Sesame::WithMessageSize<RightSize> rightSesame{ rightSerial };
        testing::StrictMock<services::SesameObserverMock> rightUpper{ rightSesame.windowed };
    };
}

class SesameInstantiationTest
    : public testing::Test
    , public infra::ClockFixture
    , public SesameInstantiationPair<256, 1024>
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
    const auto messageSize = leftUpper.Subject().MaxSendMessageSize();

    ExpectSend(leftUpper, sentData, messageSize, 'a');
    ExpectReceive(rightUpper, sentData);
    leftUpper.Subject().RequestSendMessage(messageSize);
    ExecuteAllActions();

    EXPECT_THAT(sentData.size(), testing::Eq(121));
}

TEST_F(SesameInstantiationTest, send_big_message_left)
{
    std::string sentData;
    const auto messageSize = rightUpper.Subject().MaxSendMessageSize();

    ExpectSend(rightUpper, sentData, messageSize, 'a');
    ExpectReceive(leftUpper, sentData);
    rightUpper.Subject().RequestSendMessage(messageSize);
    ExecuteAllActions();

    EXPECT_THAT(sentData.size(), testing::Eq(121));
}

class SesameInstantiationMessageSizeTest
    : public testing::TestWithParam<std::size_t>
    , public infra::ClockFixture
    , public SesameInstantiationPair<2048, 2048>
{
public:
    SesameInstantiationMessageSizeTest()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }
};

TEST_P(SesameInstantiationMessageSizeTest, send_message_of_size_right)
{
    const auto messageSize = GetParam();
    ASSERT_THAT(leftUpper.Subject().MaxSendMessageSize(), testing::Eq(1010));

    std::string sentData1;
    std::string sentData2;

    {
        testing::InSequence sequence;
        ExpectSend(leftUpper, sentData1, messageSize, 'a');
        ExpectSend(leftUpper, sentData2, messageSize, 0);
    }
    {
        testing::InSequence sequence;
        ExpectReceive(rightUpper, sentData1);
        ExpectReceive(rightUpper, sentData2);
    }
    leftUpper.Subject().RequestSendMessage(messageSize);
    leftUpper.Subject().RequestSendMessage(messageSize);
    ExecuteAllActions();

    EXPECT_THAT(sentData1.size(), testing::Eq(messageSize));
    EXPECT_THAT(sentData2.size(), testing::Eq(messageSize));
}

#ifndef EMIL_MUTATION_TESTING
INSTANTIATE_TEST_SUITE_P(SesameInstantiationMessageSize, SesameInstantiationMessageSizeTest, testing::Range<std::size_t>(1, 1011));
#else
INSTANTIATE_TEST_SUITE_P(SesameInstantiationMessageSize, SesameInstantiationMessageSizeTest, testing::Values<std::size_t>(1, 113, 225, 338, 450, 563, 675, 788, 900, 1010));
#endif
