#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/DoubleBufferedSerialCommunication.hpp"
#include "gtest/gtest.h"

class DoubleBufferedSerialCommunicationTest
    : public testing::Test
{
public:
    testing::StrictMock<hal::BufferedSerialCommunicationMock> delegate;
    services::DoubleBufferedSerialCommunication::WithStorage<4> communication{ delegate };
    hal::BufferedSerialCommunicationObserverMock observer{ communication };
};

TEST_F(DoubleBufferedSerialCommunicationTest, read_and_ack_are_delegated)
{
    infra::StreamReaderWithRewindingMock reader;
    EXPECT_CALL(delegate, Reader()).WillOnce(testing::ReturnRef(reader));
    EXPECT_EQ(&reader, &communication.Reader());

    EXPECT_CALL(delegate, AckReceived());
    communication.AckReceived();
}

TEST_F(DoubleBufferedSerialCommunicationTest, data_received_is_delegated)
{
    EXPECT_CALL(observer, DataReceived());
    delegate.GetObserver().DataReceived();
}

TEST_F(DoubleBufferedSerialCommunicationTest, send_data_is_delegated_and_reported_done_immediately_when_it_fits_in_the_buffer)
{
    testing::InSequence s;

    static const std::array<uint8_t, 4> data{ 1, 2, 3, 4 };
    testing::StrictMock<infra::MockCallback<void()>> onDone;
    infra::Function<void()> onSent;
    EXPECT_CALL(onDone, callback());
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(data), testing::_)).WillOnce(testing::SaveArg<1>(&onSent));
    communication.SendData(data, [&]()
        {
            onDone.callback();
        });

    // Verify that the onDone callback has happened before the delegate's onDone is reported
    testing::Mock::VerifyAndClearExpectations(&onDone);
    onSent();
}

TEST_F(DoubleBufferedSerialCommunicationTest, send_more_data_than_fits_in_the_buffer)
{
    testing::InSequence s;

    static const std::array<uint8_t, 6> dataSend{ 1, 2, 3, 4, 5, 6 };
    static const std::array<uint8_t, 4> dataChunk1{ 1, 2, 3, 4 };
    static const std::array<uint8_t, 2> dataChunk2{ 5, 6 };

    testing::StrictMock<infra::MockCallback<void()>> onDone;
    infra::Function<void()> onSent1;
    infra::Function<void()> onSent2;
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(dataChunk1), testing::_)).WillOnce(testing::SaveArg<1>(&onSent1));
    EXPECT_CALL(onDone, callback());
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(dataChunk2), testing::_)).WillOnce(testing::SaveArg<1>(&onSent2));
    communication.SendData(dataSend, [&]()
        {
            onDone.callback();
        });

    onSent1();
    // Verify that the onDone callback has happened before the delegate's onDone is reported
    testing::Mock::VerifyAndClearExpectations(&onDone);
    onSent2();
}

TEST_F(DoubleBufferedSerialCommunicationTest, send_second_data_after_first)
{
    testing::InSequence s;

    static const std::array<uint8_t, 4> data1{ 1, 2, 3, 4 };
    static const std::array<uint8_t, 2> data2{ 5, 6 };
    testing::StrictMock<infra::MockCallback<void()>> onDone2;
    infra::Function<void()> onSent1;
    infra::Function<void()> onSent2;
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(data1), testing::_)).WillOnce(testing::SaveArg<1>(&onSent1));
    communication.SendData(data1, [&]()
        {
            communication.SendData(data2, [&]()
                {
                    onDone2.callback();
                });
        });

    EXPECT_CALL(onDone2, callback());
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(data2), testing::_)).WillOnce(testing::SaveArg<1>(&onSent2));
    onSent1();
    testing::Mock::VerifyAndClearExpectations(&onDone2);
    onSent2();
}

TEST_F(DoubleBufferedSerialCommunicationTest, send_empty_data_is_reported_done)
{
    testing::InSequence s;

    static const std::array<uint8_t, 0> data;
    testing::StrictMock<infra::MockCallback<void()>> onDone;
    infra::Function<void()> onSent;
    EXPECT_CALL(onDone, callback());
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(data), testing::_)).WillOnce(testing::SaveArg<1>(&onSent));
    communication.SendData(data, [&]()
        {
            onDone.callback();
        });

    // Verify that the onDone callback has happened before the delegate's onDone is reported
    testing::Mock::VerifyAndClearExpectations(&onDone);
    onSent();
}

TEST_F(DoubleBufferedSerialCommunicationTest, send_second_data_while_sending_first)
{
    testing::InSequence s;

    static const std::array<uint8_t, 4> data1{ 1, 2, 3, 4 };
    static const std::array<uint8_t, 2> data2{ 5, 6 };
    testing::StrictMock<infra::MockCallback<void()>> onDone2;
    infra::Function<void()> onSent1;
    infra::Function<void()> onSent2;
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(data1), testing::_)).WillOnce(testing::SaveArg<1>(&onSent1));
    communication.SendData(data1, [&]() {});

    EXPECT_CALL(onDone2, callback());
    EXPECT_CALL(delegate, SendData(infra::ContentsEqual(data2), testing::_)).WillOnce(testing::SaveArg<1>(&onSent2));

    // Before reporting the first send done, request the second data.
    communication.SendData(data2, [&]()
        {
            onDone2.callback();
        });

    onSent1();
    testing::Mock::VerifyAndClearExpectations(&onDone2);

    onSent2();
}
