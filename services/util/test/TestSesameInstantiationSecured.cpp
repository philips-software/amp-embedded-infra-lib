#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "services/util/SesameInstantiationSecured.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class SesameInstantiationSecuredPair
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
        services::SesameSecured::KeyType keyA{ 1, 2 };
        services::SesameSecured::KeyType keyB{ 3, 4 };
        services::SesameSecured::IvType ivA{ 5, 6 };
        services::SesameSecured::IvType ivB{ 7, 8 };

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<LeftSize> leftSerial{ serial.Server() };
        typename main_::SesameInstantiationSecured<LeftSize>::WithCryptoMbedTls leftSesame{ leftSerial, services::SesameSecured::KeyMaterial{ keyA, ivA, keyB, ivB } };
        testing::StrictMock<services::SesameObserverMock> leftUpper{ leftSesame.secured };

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<RightSize> rightSerial{ serial.Client() };
        typename main_::SesameInstantiationSecured<RightSize>::WithCryptoMbedTls rightSesame{ rightSerial, services::SesameSecured::KeyMaterial{ keyB, ivB, keyA, ivA } };
        testing::StrictMock<services::SesameObserverMock> rightUpper{ rightSesame.secured };
    };
}

class SesameInstantiationSecuredTest
    : public testing::Test
    , public infra::ClockFixture
    , public SesameInstantiationSecuredPair<256, 1024>
{
public:
    SesameInstantiationSecuredTest()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }
};

TEST_F(SesameInstantiationSecuredTest, send_big_message_right)
{
    std::string sentData;
    const auto messageSize = leftUpper.Subject().MaxSendMessageSize();

    ExpectSend(leftUpper, sentData, messageSize, 'a');
    ExpectReceive(rightUpper, sentData);
    leftUpper.Subject().RequestSendMessage(messageSize);
    ExecuteAllActions();

    EXPECT_THAT(sentData.size(), testing::Eq(105));
}

TEST_F(SesameInstantiationSecuredTest, send_big_message_left)
{
    std::string sentData;
    const auto messageSize = rightUpper.Subject().MaxSendMessageSize();

    ExpectSend(rightUpper, sentData, messageSize, 'a');
    ExpectReceive(leftUpper, sentData);
    rightUpper.Subject().RequestSendMessage(messageSize);
    ExecuteAllActions();

    EXPECT_THAT(sentData.size(), testing::Eq(105));
}

class SesameInstantiationSecuredMessageSizeTest
    : public testing::TestWithParam<std::size_t>
    , public infra::ClockFixture
    , public SesameInstantiationSecuredPair<2048, 2048>
{
public:
    SesameInstantiationSecuredMessageSizeTest()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }
};

TEST_P(SesameInstantiationSecuredMessageSizeTest, send_message_of_size_right)
{
    const auto messageSize = GetParam();
    ASSERT_THAT(leftUpper.Subject().MaxSendMessageSize(), testing::Eq(994));

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
INSTANTIATE_TEST_SUITE_P(SesameInstantiationSecuredMessageSize, SesameInstantiationSecuredMessageSizeTest, testing::Range<std::size_t>(1, 995));
#else
INSTANTIATE_TEST_SUITE_P(SesameInstantiationSecuredMessageSize, SesameInstantiationSecuredMessageSizeTest, testing::Values<std::size_t>(1, 111, 221, 332, 442, 553, 663, 774, 884, 994));
#endif
