#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SesameSecured.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"

#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "services/util/SesameInstantiationSecured.hpp"

class SesameInstantiationSecuredTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    SesameInstantiationSecuredTest()
    {
        EXPECT_CALL(leftUpper, Initialized()).Times(testing::AnyNumber());
        EXPECT_CALL(rightUpper, Initialized()).Times(testing::AnyNumber());
        ExecuteAllActions();
    }

    services::SerialCommunicationLoopback serial;
    services::SesameSecured::KeyType keyA{ 1, 2 };
    services::SesameSecured::KeyType keyB{ 3, 4 };
    services::SesameSecured::IvType ivA{ 5, 6 };
    services::SesameSecured::IvType ivB{ 7, 8 };

    constexpr static std::size_t leftSize = 256;
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<leftSize> leftSerial{ serial.Server() };
    main_::SesameSecured::WithMessageSize<leftSize> leftSesame{ leftSerial, services::SesameSecured::KeyMaterial{ keyA, ivA, keyB, ivB } };
    testing::StrictMock<services::SesameObserverMock> leftUpper{ leftSesame.secured };

    constexpr static std::size_t rightSize = 1024;
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<rightSize> rightSerial{ serial.Client() };
    main_::SesameSecured::WithMessageSize<rightSize> rightSesame{ rightSerial, services::SesameSecured::KeyMaterial{ keyB, ivB, keyA, ivA } };
    testing::StrictMock<services::SesameObserverMock> rightUpper{ rightSesame.secured };
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
            stream >> infra::BoundedString(text);
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
            stream >> infra::BoundedString(text);
            EXPECT_EQ(sentData, text);
        }));
    ExecuteAllActions();

    EXPECT_EQ(105, sentData.size());
}
