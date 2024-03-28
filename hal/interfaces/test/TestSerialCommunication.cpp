#include "hal/interfaces/SerialCommunication.hpp"
#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gtest/gtest.h"

class BufferedSerialCommunicationOnUnbufferedTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    infra::Function<void(infra::ConstByteRange data)> receivedData;
    testing::StrictMock<hal::SerialCommunicationCleanMock> serial;
    infra::Execute execute{
        [this]()
        {
            EXPECT_CALL(serial, ReceiveData(testing::_)).WillOnce(testing::SaveArg<0>(&receivedData));
        }
    };
    hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<4> buffered{ serial };
    testing::StrictMock<hal::BufferedSerialCommunicationObserverMock> observer{ buffered };
    infra::ExecuteOnDestruction executeOnDestruction{
        [this]()
        {
            EXPECT_CALL(serial, ReceiveData(testing::_));
        }
    };

    infra::StreamErrorPolicy errorPolicy;
};

TEST_F(BufferedSerialCommunicationOnUnbufferedTest, SendData_is_forwarded)
{
    infra::VerifyingFunctionMock<void()> onDone;
    std::array<uint8_t, 4> data;

    EXPECT_CALL(serial, SendData(infra::MakeConstByteRange(data), testing::_)).WillOnce(testing::InvokeArgument<1>());
    buffered.SendData(data, [&]()
        {
            onDone.callback();
        });
}

TEST_F(BufferedSerialCommunicationOnUnbufferedTest, ReceivedData_is_available_through_Reader)
{
    std::array<uint8_t, 4> data{ 0, 1, 2, 3 };
    receivedData(data);

    auto& reader = buffered.Reader();
    std::array<uint8_t, 4> extracted;
    reader.Extract(extracted, errorPolicy);
    EXPECT_EQ(data, extracted);
}

TEST_F(BufferedSerialCommunicationOnUnbufferedTest, after_AckReceived_space_is_available)
{
    std::array<uint8_t, 4> data{ 0, 1, 2, 3 };
    receivedData(data);

    auto& reader = buffered.Reader();
    ASSERT_EQ(4, reader.Available());
    reader.ExtractContiguousRange(4);
    buffered.AckReceived();
    EXPECT_EQ(0, reader.Available());
    receivedData(data);
}

TEST_F(BufferedSerialCommunicationOnUnbufferedTest, received_data_results_in_notification)
{
    std::array<uint8_t, 2> data{ 0, 1 };
    receivedData(data);
    receivedData(data);

    EXPECT_CALL(observer, DataReceived());
    ExecuteAllActions();
}

TEST_F(BufferedSerialCommunicationOnUnbufferedTest, detached_observer_is_not_notified)
{
    std::array<uint8_t, 2> data{ 0, 1 };
    receivedData(data);

    observer.Detach();
    ExecuteAllActions();
}
