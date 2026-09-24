#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "hal/synchronous_interfaces/test_doubles/SynchronousSerialCommunicationMock.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/MultipurposeSerialCommunicationAdapter.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <optional>
#include <utility>

namespace
{
    class TestFixture
        : public hal::MultpurposeSerialCommuniationAdapter<testing::StrictMock<hal::SynchronousSerialCommunicationMock>, testing::StrictMock<hal::SerialCommunicationMock>>
    {
    public:
        using Synchronous = testing::StrictMock<hal::SynchronousSerialCommunicationMock>;
        using Asynchronous = testing::StrictMock<hal::SerialCommunicationMock>;

        TestFixture()
            : hal::MultpurposeSerialCommuniationAdapter<Synchronous, Asynchronous>(std::in_place_type<Synchronous>)
        {
        }

        Asynchronous& AsynchronousMock()
        {
            return Get<Asynchronous>();
        }

        Synchronous& SynchronousMock()
        {
            return Get<Synchronous>();
        }

        void ExpectAsyncSend(std::vector<uint8_t> data)
        {
            if (IsSynchronous())
                expectedAsyncSend = std::move(data);
            else
                EXPECT_CALL(AsynchronousMock(), SendDataMock(data));
        }

        void ExpectSyncSend(std::vector<uint8_t> data)
        {
            if (IsSynchronous())
                EXPECT_CALL(SynchronousMock(), SendDataMock(data));
            else
                expectedSyncSend = std::move(data);
        }

        void ExpectSyncReceive(std::vector<uint8_t> data)
        {
            if (IsSynchronous())
                EXPECT_CALL(SynchronousMock(), ReceiveDataMock()).WillOnce(testing::Return(std::make_pair(true, data)));
            else
                expectedSyncReceive = std::move(data);
        }

        std::size_t asynchronousEmplacements = 0;
        std::size_t synchronousEmplacements = 1;

    private:
        using hal::MultpurposeSerialCommuniationAdapter<Synchronous, Asynchronous>::IsSynchronous;

        void EmplaceAsync() override
        {
            Emplace<Asynchronous>();
            ++asynchronousEmplacements;
            if (expectedAsyncSend)
            {
                EXPECT_CALL(AsynchronousMock(), SendDataMock(*expectedAsyncSend));
                expectedAsyncSend.reset();
            }
        }

        void EmplaceSync() override
        {
            Emplace<Synchronous>();
            ++synchronousEmplacements;
            if (expectedSyncSend)
            {
                EXPECT_CALL(SynchronousMock(), SendDataMock(*expectedSyncSend));
                expectedSyncSend.reset();
            }
            if (expectedSyncReceive)
            {
                EXPECT_CALL(SynchronousMock(), ReceiveDataMock()).WillOnce(testing::Return(std::make_pair(true, *expectedSyncReceive)));
                expectedSyncReceive.reset();
            }
        }

        std::optional<std::vector<uint8_t>> expectedAsyncSend;
        std::optional<std::vector<uint8_t>> expectedSyncSend;
        std::optional<std::vector<uint8_t>> expectedSyncReceive;
    };
}

class MultipurposeSerialCommunicationAdapterTest
    : public testing::Test
{
public:
    TestFixture adapter;
};

TEST_F(MultipurposeSerialCommunicationAdapterTest, send_data_asynchronously_switches_from_synchronous)
{
    infra::MockCallback<void()> actionOnCompletion;
    std::vector<uint8_t> data{ 0xfe, 0xff };
    adapter.ExpectAsyncSend(data);

    adapter.SendData(data, [&actionOnCompletion]
        {
            actionOnCompletion.callback();
        });
    EXPECT_CALL(actionOnCompletion, callback());
    adapter.AsynchronousMock().actionOnCompletion();

    EXPECT_THAT(adapter.asynchronousEmplacements, testing::Eq(1));
    EXPECT_THAT(adapter.synchronousEmplacements, testing::Eq(1));
}

TEST_F(MultipurposeSerialCommunicationAdapterTest, receive_data_asynchronously_switches_from_synchronous)
{
    infra::ConstByteRange receivedData;
    std::vector<uint8_t> data{ 0xfe, 0xff };

    adapter.ReceiveData([&receivedData](infra::ConstByteRange received)
        {
            receivedData = received;
        });
    adapter.AsynchronousMock().dataReceived(data);

    EXPECT_THAT(receivedData, testing::ElementsAre(0xfe, 0xff));
    EXPECT_THAT(adapter.asynchronousEmplacements, testing::Eq(1));
    EXPECT_THAT(adapter.synchronousEmplacements, testing::Eq(1));
}

TEST_F(MultipurposeSerialCommunicationAdapterTest, send_data_synchronously_forwards_to_synchronous_mock)
{
    std::vector<uint8_t> data{ 0xfe, 0xff };
    adapter.ExpectSyncSend(data);

    adapter.SendData(data);

    EXPECT_THAT(adapter.synchronousEmplacements, testing::Eq(1));
}

TEST_F(MultipurposeSerialCommunicationAdapterTest, synchronous_send_after_asynchronous_use_switches_back_to_synchronous)
{
    infra::MockCallback<void()> actionOnCompletion;
    adapter.ExpectAsyncSend(std::vector<uint8_t>{ 0x01 });
    adapter.SendData(std::vector<uint8_t>{ 0x01 }, [&actionOnCompletion]
        {
            actionOnCompletion.callback();
        });
    EXPECT_CALL(actionOnCompletion, callback());
    adapter.AsynchronousMock().actionOnCompletion();
    adapter.ExpectSyncSend(std::vector<uint8_t>{ 0xfe, 0xff });

    adapter.SendData(std::vector<uint8_t>{ 0xfe, 0xff });

    EXPECT_THAT(adapter.asynchronousEmplacements, testing::Eq(1));
    EXPECT_THAT(adapter.synchronousEmplacements, testing::Eq(2));
}

TEST_F(MultipurposeSerialCommunicationAdapterTest, receive_data_synchronously_forwards_result_and_data)
{
    std::vector<uint8_t> data{ 0xfe, 0xff };
    adapter.ExpectSyncReceive(data);
    std::vector<uint8_t> receivedData(2);

    EXPECT_THAT(adapter.ReceiveData(infra::MakeRange(receivedData)), testing::IsTrue());
    EXPECT_THAT(receivedData, testing::ElementsAre(0xfe, 0xff));
}

TEST_F(MultipurposeSerialCommunicationAdapterTest, synchronous_receive_after_asynchronous_use_switches_back_to_synchronous)
{
    adapter.ReceiveData([](infra::ConstByteRange) {});

    std::vector<uint8_t> data{ 0xfe, 0xff };
    adapter.ExpectSyncReceive(data);
    std::vector<uint8_t> receivedData(2);

    EXPECT_THAT(adapter.ReceiveData(infra::MakeRange(receivedData)), testing::IsTrue());
    EXPECT_THAT(receivedData, testing::ElementsAre(0xfe, 0xff));
    EXPECT_THAT(adapter.asynchronousEmplacements, testing::Eq(1));
    EXPECT_THAT(adapter.synchronousEmplacements, testing::Eq(2));
}
