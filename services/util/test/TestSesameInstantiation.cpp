#include "infra/stream/StdVectorInputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "services/util/SesameInstantiation.hpp"
#include "services/util/test_doubles/SesameMock.hpp"
#include "gmock/gmock.h"
#include <vector>

namespace
{
    template<std::size_t LeftSize, std::size_t RightSize>
    class SesameInstantiation
    {
    public:
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

#ifndef EMIL_MUTATION_TESTING
INSTANTIATE_TEST_SUITE_P(SesameInstantiationTestMessageSize, SesameInstantiationTestMessageSize, testing::Range<std::size_t>(1, 1011));
#else
INSTANTIATE_TEST_SUITE_P(SesameInstantiationTestMessageSize, SesameInstantiationTestMessageSize, testing::Values<std::size_t>(1, 113, 225, 338, 450, 563, 675, 788, 900, 1010));
#endif

namespace
{
    struct StandardCobsVector
    {
        std::size_t pduSize;
        std::vector<std::size_t> zeroIndices;
        std::size_t frameSize;
    };
}

class SesameInstantiationStandardCobsPeerTest
    : public testing::TestWithParam<StandardCobsVector>
    , public infra::ClockFixture
{
public:
    static constexpr uint8_t initResponseOperation = 2;
    static constexpr uint8_t releaseWindowOperation = 3;
    static constexpr uint8_t messageOperation = 4;
    static constexpr uint16_t peerWindow = 0x0101;
    static constexpr std::size_t initResponseFrameSize = 5;

    static std::vector<uint8_t> StandardCobsFrame(const std::vector<uint8_t>& pdu)
    {
        std::vector<uint8_t> frame{ 0 };
        std::size_t codeIndex = 0;
        uint8_t code = 1;

        for (std::size_t i = 0; i != pdu.size(); ++i)
        {
            if (pdu[i] == 0)
            {
                frame[codeIndex] = code;
                codeIndex = frame.size();
                frame.push_back(0);
                code = 1;
            }
            else
            {
                frame.push_back(pdu[i]);
                ++code;

                if (code == 0xff && i + 1 != pdu.size())
                {
                    frame[codeIndex] = code;
                    codeIndex = frame.size();
                    frame.push_back(0);
                    code = 1;
                }
            }
        }

        frame[codeIndex] = code;
        frame.push_back(0);
        return frame;
    }

    static std::vector<uint8_t> WindowPacket(uint8_t operation, uint16_t window)
    {
        return { operation, static_cast<uint8_t>(window & 0xff), static_cast<uint8_t>(window >> 8) };
    }

    static std::vector<uint8_t> UserMessagePdu(const StandardCobsVector& vector)
    {
        std::vector<uint8_t> pdu(vector.pduSize, 1);
        pdu.front() = messageOperation;

        for (auto zeroIndex : vector.zeroIndices)
            pdu[zeroIndex] = 0;

        return pdu;
    }

    SesameInstantiationStandardCobsPeerTest()
    {
        peerSerial.ReceiveData([this](infra::ConstByteRange data)
            {
                receivedFromNode.insert(receivedFromNode.end(), data.begin(), data.end());
            });

        EXPECT_CALL(upper, Initialized());
        ExecuteAllActions();
        SendToNode(StandardCobsFrame(WindowPacket(initResponseOperation, peerWindow)));
        receivedFromNode.clear();
    }

    void SendToNode(const std::vector<uint8_t>& frame)
    {
        sentToNode = frame;
        peerSerial.SendData(sentToNode, []() {});
        ExecuteAllActions();
    }

    services::SerialCommunicationLoopback serial;
    hal::SerialCommunication& peerSerial{ serial.Client() };
    std::vector<uint8_t> sentToNode;
    std::vector<uint8_t> receivedFromNode;

    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<2048> nodeSerial{ serial.Server() };
    main_::Sesame::WithMessageSize<1024, 8> sesame{ nodeSerial };
    testing::StrictMock<services::SesameObserverMock> upper{ sesame.windowed };
};

TEST_P(SesameInstantiationStandardCobsPeerTest, node_releases_exactly_the_encoded_bytes_received)
{
    const auto vector = GetParam();
    const auto messageFrame = StandardCobsFrame(UserMessagePdu(vector));
    ASSERT_THAT(messageFrame, testing::SizeIs(vector.frameSize));

    EXPECT_CALL(upper, ReceivedMessage(testing::_));
    SendToNode(messageFrame);

    const auto releasedWindow = static_cast<uint16_t>(initResponseFrameSize + vector.frameSize);
    EXPECT_THAT(receivedFromNode, testing::ElementsAreArray(StandardCobsFrame(WindowPacket(releaseWindowOperation, releasedWindow))));
}

INSTANTIATE_TEST_SUITE_P(SesameInstantiationStandardCobsPeerTest, SesameInstantiationStandardCobsPeerTest,
    testing::Values(
        StandardCobsVector{ 253, {}, 255 },
        StandardCobsVector{ 254, {}, 256 },
        StandardCobsVector{ 255, {}, 258 },
        StandardCobsVector{ 255, { 254 }, 258 },
        StandardCobsVector{ 300, { 100 }, 302 },
        StandardCobsVector{ 507, {}, 510 },
        StandardCobsVector{ 508, {}, 511 },
        StandardCobsVector{ 509, {}, 513 },
        StandardCobsVector{ 509, { 254 }, 512 },
        StandardCobsVector{ 509, { 100, 300 }, 511 },
        StandardCobsVector{ 600, { 254 }, 604 }));
