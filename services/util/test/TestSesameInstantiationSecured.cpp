#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "services/util/SesameInstantiationSecured.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class SesameInstantiationSecured
    {
    public:
        services::SerialCommunicationLoopback serial;
        services::SesameSecured::KeyType keyA{ 1, 2 };
        services::SesameSecured::KeyType keyB{ 3, 4 };
        services::SesameSecured::IvType ivA{ 5, 6 };
        services::SesameSecured::IvType ivB{ 7, 8 };

        constexpr static std::size_t leftSize = LeftSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<leftSize> leftSerial{ serial.Server() };
        main_::SesameSecured::WithMessageSize<leftSize> leftSesame{ leftSerial, services::SesameSecured::KeyMaterial{ keyA, ivA, keyB, ivB } };
        testing::StrictMock<services::SesameObserverMock> leftUpper{ leftSesame.secured };

        constexpr static std::size_t rightSize = RightSize;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<rightSize> rightSerial{ serial.Client() };
        main_::SesameSecured::WithMessageSize<rightSize> rightSesame{ rightSerial, services::SesameSecured::KeyMaterial{ keyB, ivB, keyA, ivA } };
        testing::StrictMock<services::SesameObserverMock> rightUpper{ rightSesame.secured };
    };
}

class SesameInstantiationSecuredTest
    : public testing::Test
    , public infra::ClockFixture
    , public SesameInstantiationSecured<256, 1024>
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

    EXPECT_EQ(105, sentData.size());
}

TEST_F(SesameInstantiationSecuredTest, send_big_message_left)
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

    EXPECT_EQ(105, sentData.size());
}

class SesameInstantiationSecuredTestMessageSize
    : public testing::TestWithParam<std::size_t>
    , public infra::ClockFixture
    , public SesameInstantiationSecured<2048, 2048>
{
public:
    SesameInstantiationSecuredTestMessageSize()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }

    const std::size_t messageSize = GetParam();
};

TEST_P(SesameInstantiationSecuredTestMessageSize, send_message_of_size_right)
{
    EXPECT_EQ(994, leftUpper.Subject().MaxSendMessageSize());

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

INSTANTIATE_TEST_SUITE_P(SesameInstantiationSecuredTestMessageSize, SesameInstantiationSecuredTestMessageSize, testing::Range<std::size_t>(1, 994));
