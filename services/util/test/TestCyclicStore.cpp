#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/CyclicStore.hpp"

class CyclicStoreBaseTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CyclicStoreBaseTest()
        : flash(2, 10)
    {}

    hal::FlashStub flash;
};

class CyclicStoreTest
    : public CyclicStoreBaseTest
{
public:
    CyclicStoreTest()
        : cyclicStore(flash)
    {}

    services::CyclicStore cyclicStore;
};

TEST_F(CyclicStoreTest, AddFirstItem)
{
    infra::MockCallback<void()> done;

    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, [&done]() { done.callback(); });

    EXPECT_CALL(done, callback());
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, AddPartialItem)
{
    std::vector<uint8_t> data = { 11 };
    cyclicStore.AddPartial(data, 3, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 12 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> thirdData = { 13 };
    cyclicStore.Add(thirdData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 3, 0, 11, 12, 13, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, AddSecondItem)
{
    std::vector<uint8_t> data = { 11 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 12 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 1, 0, 11, 0xf8, 1, 0, 12, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, FillSector)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);
}

TEST_F(CyclicStoreTest, AddItemInNewSector)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
        { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);
}

TEST_F(CyclicStoreTest, FillUnusedSpaceWhenAddingItem)
{
    std::vector<uint8_t> data = { 11, 12, 13 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 3, 0, 11, 12, 13, 0x7f, 0xff, 0xff },
        { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);
}

TEST_F(CyclicStoreTest, FillOneByteOfUnusedSpaceWhenAddingItem)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 5, 0, 11, 12, 13, 14, 15, 0x7f, },
        { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);
}

TEST_F(CyclicStoreTest, AddFirstItemWhenFlashStopsAfterNSteps)
{
    std::vector<std::vector<uint8_t>> flashAfterNSteps;

    for (uint8_t i = 1; i != 7; ++i)
    {
        cyclicStore.Clear(infra::emptyFunction);
        ExecuteAllActions();
        flash.stopAfterWriteSteps = i;

        std::vector<uint8_t> data = { 11, 12 };
        cyclicStore.Add(data, infra::emptyFunction);
        ExecuteAllActions();

        flashAfterNSteps.push_back(flash.sectors[0]);
    }

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xfe,    2,    0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xfc,    2,    0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xfc,    2,    0,   11,   12, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xf8,    2,    0,   11,   12, 0xff, 0xff, 0xff, 0xff }
    }), flashAfterNSteps);
}

TEST_F(CyclicStoreTest, AddItemWhichCausesSectorToBeErased)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21, 22, 23, 24, 25, 26 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> thirdData = { 31, 32, 33, 34, 35, 36 };
    cyclicStore.Add(thirdData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
        { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
    }), flash.sectors);
}

TEST_F(CyclicStoreTest, AddItemOverflowingSectorWhichCausesSectorToBeErased)
{
    flash.sectors[0].resize(14);
    flash.sectors[1].resize(14);
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21, 22, 23, 24, 25, 26 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> thirdData = { 31, 32, 33, 34, 35, 36 };
    cyclicStore.Add(thirdData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26, 0x7f, 0xff, 0xff, 0xff },
    }), flash.sectors);
}

TEST_F(CyclicStoreTest, ReadFirstItem)
{
    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadFirstItemTwice)
{
    // build
    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();

    // operate/check
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12 }));

    iterator = cyclicStore.Begin();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadItemWithInsufficientBufferSpace)
{
    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(1, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadSecondItem)
{
    std::vector<uint8_t> data = { 11 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(20, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadSecondItemInNewSector)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
        { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadSecondItemAfterSkipSpace)
{
    std::vector<uint8_t> data = { 11, 12, 13 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 3, 0, 11, 12, 13, 0x7f, 0xff, 0xff },
        { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadSecondItemAfterSkipSpaceOfOneByte)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 5, 0, 11, 12, 13, 14, 15, 0x7f, },
        { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
    }), flash.sectors);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadSecondItemAfterSkipSpaceReallySkipsSector)
{
    flash.sectors = std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 1, 0, 11, 0x7f, 0xf8, 1, 0, 21 },
        { 0xfe, 0xf8, 1, 0, 31 },
    };

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 31 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadInEmptyYieldsEmptyRange)
{
    infra::MockCallback<void(bool)> mock;
    EXPECT_CALL(mock, callback(true));

    std::vector<uint8_t> readDataBuffer(2, 0);
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(result.empty()); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadPastEndYieldsEmptyRange)
{
    std::vector<uint8_t> data = { 11, 12, 13 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void(bool)> mock;
    EXPECT_CALL(mock, callback(true));

    std::vector<uint8_t> readDataBuffer(2, 0);
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(result.empty()); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadPastEndInAFullFlashYieldsEmptyRange)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void(bool)> mock;
    EXPECT_CALL(mock, callback(true));

    std::vector<uint8_t> readDataBuffer(6, 0);
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) {});
    ExecuteAllActions();
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(result.empty()); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadWhileAddWaitsForAddToFinish)
{
    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, infra::emptyFunction);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, AddWhileClearWaitsforClearToFinish)
{
    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    infra::MockCallback<void()> done;
    flash.delaySignalEraseDone = true;
    cyclicStore.Clear([&done]() { done.callback(); });
    infra::Function<void()> onEraseDone = flash.onEraseDone;
    ExecuteAllActions();
    flash.delaySignalEraseDone = false;

    std::vector<uint8_t> secondData = { 21, 22 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(done, callback());
    onEraseDone();

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21, 22 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, ReadItemAfterCyclicalLogging)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21, 22, 23, 24, 25, 26 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> thirdData = { 31, 32, 33, 34, 35, 36 };
    cyclicStore.Add(thirdData, infra::emptyFunction);
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
        { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
    }), flash.sectors);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }));
    
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(10, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreBaseTest, ReadItemAfterCyclicalLogging2)
{
    std::vector<std::vector<uint8_t>> sectors =
        { { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
          { 0xfe, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 } };
    flash.sectors = sectors;
    services::CyclicStore cyclicStore(flash);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12, 13, 14, 15, 16 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(10, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();

    std::vector<uint8_t> data = { 31, 32, 33, 34, 35, 36 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
        { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 }
    }), flash.sectors);

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }));

    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreBaseTest, RecoverAndReadFirstItem)
{
    {
        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        std::vector<uint8_t> data = { 11, 12 };
        cyclicStore.Add(data, infra::emptyFunction);
        ExecuteAllActions();
    }

    services::CyclicStore cyclicStore(flash);
    ExecuteAllActions();

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    ExecuteAllActions();
}

TEST_F(CyclicStoreBaseTest, RecoverFromAllDifferentGoodScenarios)
{
    struct Scenario
    {
        std::vector<uint8_t> expected;
        std::vector<std::vector<uint8_t>> flashContents;
        std::vector<std::vector<uint8_t>> flashContentsAfterAddition;
    };

    const std::vector<uint8_t> first = { 0xfc, 0xf8, 1, 0, 55 };
    const std::vector<uint8_t> empty = { 0xff, 0xff, 0xff, 0xff, 0xff };
    const std::vector<uint8_t> used = { 0xfe, 0xf8, 1, 0, 44 };
    const std::vector<uint8_t> written = { 0xfe, 0xf8, 1, 0, 66 };
    const std::vector<uint8_t> usedAsFirst = { 0xfc, 0xf8, 1, 0, 44 };

    const std::array<Scenario, 10> scenarios = { {
        {
            std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, empty },
            std::vector<std::vector<uint8_t>>{ first, written }
        },
        {
            std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, used, empty },
            std::vector<std::vector<uint8_t>>{ first, used, written }
        },
        {
            std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, used, empty, empty },
            std::vector<std::vector<uint8_t>>{ first, used, written, empty }
        },
        {
            std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, used, used },
            std::vector<std::vector<uint8_t>>{ written, usedAsFirst, used }
        },
        {
            std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ used, empty, used },
            std::vector<std::vector<uint8_t>>{ used, written, usedAsFirst }
        },
        {
            std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ empty, used, used},
            std::vector<std::vector<uint8_t>>{ written, usedAsFirst, used }
        },
        {
            std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ used, empty },
            std::vector<std::vector<uint8_t>>{ usedAsFirst, written }
        },
        {
            std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ used, first, used },
            std::vector<std::vector<uint8_t>>{ used, written, usedAsFirst }
        },
        {
            std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ used, used, first },
            std::vector<std::vector<uint8_t>>{ usedAsFirst, used, written }
        },
        {
            std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ used, used, empty, used, used },
            std::vector<std::vector<uint8_t>>{ used, used, written, usedAsFirst, used }
        }
    } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors = scenarios[index].flashContents;

        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        infra::MockCallback<void(std::vector<uint8_t>, std::size_t)> mock;
        EXPECT_CALL(mock, callback(scenarios[index].expected, index));

        services::CyclicStore::Iterator iterator = cyclicStore.Begin();
        std::vector<uint8_t> readDataBuffer(10, 0);
        iterator.Read(readDataBuffer, [&mock, index](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end()), index); });
        ExecuteAllActions();

        std::vector<uint8_t> data = { 66 };
        cyclicStore.Add(data, infra::emptyFunction);
        ExecuteAllActions();

        EXPECT_EQ(scenarios[index].flashContentsAfterAddition, flash.sectors);
    }
}

TEST_F(CyclicStoreBaseTest, InAllCorruptScenariosFlashIsErased)
{
    const std::vector<uint8_t> first = { 0xfc, 0xf8, 1, 0, 55 };
    const std::vector<uint8_t> empty = { 0xff, 0xff, 0xff, 0xff, 0xff };
    const std::vector<uint8_t> used = { 0xfe, 0xf8, 1, 0, 44 };
    const std::vector<uint8_t> corrupt = { 0xaa, 0xf8, 1, 0, 44 };


    const std::array<std::vector<std::vector<uint8_t>>, 8> scenarios = { {
        std::vector<std::vector<uint8_t>>{ empty, first, empty },
        std::vector<std::vector<uint8_t>>{ used, first, empty },
        std::vector<std::vector<uint8_t>>{ empty, first, used },
        std::vector<std::vector<uint8_t>>{ empty, used, used, empty },
        std::vector<std::vector<uint8_t>>{ empty, used, empty, used },
        std::vector<std::vector<uint8_t>>{ used, empty, used, empty },
        std::vector<std::vector<uint8_t>>{ used, used, used },
        std::vector<std::vector<uint8_t>>{ first, used, corrupt },
    } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors = scenarios[index];

        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        std::vector<std::vector<uint8_t>> emptyFlash = flash.sectors;
        for (auto& sector : emptyFlash)
            sector = std::vector<uint8_t>(sector.size(), 0xff);

        EXPECT_EQ(emptyFlash, flash.sectors);
    }
}

TEST_F(CyclicStoreBaseTest, AfterRecoveryItemsAreInsertedAfterCurrentContents)
{
    flash.sectors = std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 1, 0, 16, 0xff, 0xff, 0xff, 0xff },
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    };

    services::CyclicStore cyclicStore(flash);
    ExecuteAllActions();

    std::vector<uint8_t> data = { 32 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 1, 0, 16, 0xf8, 1, 0, 32 },
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    }), flash.sectors);
}

TEST_F(CyclicStoreBaseTest, RecoveryAfterPartialWriteContinuesAtTheCorrectPosition)
{
    struct Scenario
    {
        std::vector<uint8_t> flashContents;
        std::vector<uint8_t> flashContentsAfterAdd;
    };

    const std::array<Scenario, 5> scenarios =
    { {
        {
            { 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
            { 0xfc, 0xf8, 1, 0, 22, 0xff, 0xff, 0xff, 0xff, 0xff },
        },
        {
            { 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
            { 0xfc, 0xfe, 0xff, 0xff, 0xf8, 1, 0, 22, 0xff, 0xff },
        },
        {
            { 0xfc, 0xfe, 2, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
            { 0xfc, 0xfe, 2, 0, 0xf8, 1, 0, 22, 0xff, 0xff },
        },
        {
            { 0xfc, 0xfc, 2, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
            { 0xfc, 0xfc, 2, 0, 0xff, 0xff, 0xf8, 1, 0, 22 },
        },
        {
            { 0xfc, 0xfc, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff },
            { 0xfc, 0xfc, 2, 0, 11, 12, 0xf8, 1, 0, 22 },
        }
    } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index].flashContents;

        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        std::vector<uint8_t> data = { 22 };
        cyclicStore.Add(data, infra::emptyFunction);
        ExecuteAllActions();

        EXPECT_EQ(scenarios[index].flashContentsAfterAdd, flash.sectors[0]);
    }
}

TEST_F(CyclicStoreBaseTest, RecoveryAfterCorruptionContinuesAtTheNextSector)
{
    const std::array<std::vector<uint8_t>, 3> scenarios =
    { {
        { 0xfc, 0xaa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xf8, 0xaa, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xfc, 2, 0, 11, 12, 0xaa, 0xff, 0xff, 0xff }
    } };

    const std::vector<uint8_t> nextSector = { 0xfe, 0xf8, 1, 0, 22, 0xff, 0xff, 0xff, 0xff, 0xff };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index];
        flash.sectors[1] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        std::vector<uint8_t> data = { 22 };
        cyclicStore.Add(data, infra::emptyFunction);
        ExecuteAllActions();

        EXPECT_EQ(nextSector, flash.sectors[1]);
    }
}

TEST_F(CyclicStoreBaseTest, ReadSkipsIncompleteData)
{
    const std::array<std::vector<uint8_t>, 4> scenarios =
    { {
            { 0xfc, 0xfe, 0xff, 0xff, 0xf8, 1, 0, 22, 0xff, 0xff },
            { 0xfc, 0xfe, 2, 0, 0xf8, 1, 0, 22, 0xff, 0xff },
            { 0xfc, 0xfc, 2, 0, 0xff, 0xff, 0xf8, 1, 0, 22 },
            { 0xfc, 0xfc, 2, 0, 11, 12, 0xf8, 1, 0, 22 },
    } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index];

        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        infra::MockCallback<void(std::vector<uint8_t>)> mock;
        EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 22 }));

        services::CyclicStore::Iterator iterator = cyclicStore.Begin();
        std::vector<uint8_t> readDataBuffer(2, 0);
        iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
        ExecuteAllActions();
    }
}

TEST_F(CyclicStoreBaseTest, OnCorruptDataReadSkipsToNextSector)
{
    const std::array<std::vector<uint8_t>, 3> scenarios =
    { {
        { 0xfc, 0xaa, 0xf8, 1, 0, 22 },
        { 0xfc, 0xf8, 0xaa, 0, 0xf8, 1, 0, 22 },
        { 0xfc, 0xfc, 2, 0, 11, 12, 0xaa, 0xf8, 1, 0, 22 }
    } };

    flash.sectors[1] = { 0xfe, 0xf8, 1, 0, 33 };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index];

        services::CyclicStore cyclicStore(flash);
        ExecuteAllActions();

        infra::MockCallback<void(std::vector<uint8_t>)> mock;
        EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 33 }));

        services::CyclicStore::Iterator iterator = cyclicStore.Begin();
        std::vector<uint8_t> readDataBuffer(2, 0);
        iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
        ExecuteAllActions();
    }
}

TEST_F(CyclicStoreTest, WhenSectorIsErasedIteratorIsMovedForward)
{
    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 21, 22, 23, 24, 25, 26 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
        { 0xfe, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
    }), flash.sectors);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();

    std::vector<uint8_t> erasingSectorData = { 31 };
    cyclicStore.Add(erasingSectorData, infra::emptyFunction);
    ExecuteAllActions();

    std::vector<uint8_t> readDataBuffer(12, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, TestParallelAddAndClear)
{
    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(6, 0);

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{}));
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);

    cyclicStore.Clear(infra::emptyFunction);

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, TestParallelAddPartialAndClear)
{
    std::vector<uint8_t> data = { 11 };
    cyclicStore.AddPartial(data, 3, infra::emptyFunction);
    cyclicStore.Clear(infra::emptyFunction);
    ExecuteAllActions();
    std::vector<uint8_t> secondData = { 12 };
    cyclicStore.Add(secondData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xfc, 3, 0, 11, 12, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    std::vector<uint8_t> thirdData = { 13 };
    cyclicStore.Add(thirdData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, TestParallelAddAndClearUrgent)
{
    std::vector<uint8_t> data = { 11 };
    cyclicStore.Add(data, infra::emptyFunction);
    ExecuteAllActions();

    std::vector<uint8_t> data2 = { 12 };
    cyclicStore.Add(data2, infra::emptyFunction);
    cyclicStore.ClearUrgent(infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 1, 0, 12, 0xff, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, when_iterator_is_at_start_of_an_empty_sector_newly_added_item_is_available_in_next_read)
{
    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(6, 0);

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12, 13, 14, 15, 16 }));

    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }));

    std::vector<uint8_t> data2 = { 21, 22, 23, 24, 25, 26 };
    cyclicStore.Add(data2, infra::emptyFunction);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}

TEST_F(CyclicStoreBaseTest, when_iterator_is_at_the_end_of_flash_newly_added_item_is_available_in_next_read)
{
    hal::FlashStub flash(2, 30);
    services::CyclicStore cyclicStore(flash);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(6, 0);

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12, 13, 14, 15, 16 }));

    std::vector<uint8_t> data = { 11, 12, 13, 14, 15, 16 };
    cyclicStore.Add(data, infra::emptyFunction);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();

    EXPECT_CALL(mock, callback(std::vector<uint8_t>()));
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }));

    std::vector<uint8_t> data2 = { 21, 22, 23, 24, 25, 26 };
    cyclicStore.Add(data2, infra::emptyFunction);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result) { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });
    ExecuteAllActions();
}
