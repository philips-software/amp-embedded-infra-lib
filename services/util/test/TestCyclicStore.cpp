#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/LifetimeHelper.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/CyclicStore.hpp"
#include "gtest/gtest.h"

class CyclicStoreTest
    : public testing::Test
    , public infra::ClockFixture
    , protected infra::LifetimeHelper
{
public:
    void AddItem(infra::ConstByteRange data, const infra::Function<void()>& onDone = infra::emptyFunction)
    {
        cyclicStore.Add(data, onDone);
        ExecuteAllActions();
    }

    void AddPartialItem(infra::ConstByteRange data, uint32_t totalSize, const infra::Function<void()>& onDone = infra::emptyFunction)
    {
        cyclicStore.AddPartial(data, totalSize, onDone);
        ExecuteAllActions();
    }

    void ReConstructCyclicStore()
    {
        infra::ReConstruct(cyclicStore, flash);
        ExecuteAllActions();
    }

    void ReConstructFlashAndCyclicStore(uint32_t numberOfSectors, uint32_t sizeOfEachSector)
    {
        infra::ReConstruct(flash, numberOfSectors, sizeOfEachSector);
        ReConstructCyclicStore();
    }

    std::vector<uint8_t> Read(services::CyclicStore::Iterator& iterator, std::size_t maxSize = 1000)
    {
        std::vector<uint8_t> result;

        std::vector<uint8_t> readDataBuffer(maxSize, 0);
        iterator.Read(readDataBuffer, [&](infra::ByteRange data)
            { result.insert(result.end(), data.begin(), data.end()); });
        ExecuteAllActions();

        return result;
    }

    hal::FlashStub flash{ 2, 10 };
    services::CyclicStore cyclicStore{ flash };
};

TEST_F(CyclicStoreTest, AddFirstItem)
{
    infra::VerifyingFunctionMock<void()> done;

    AddItem(KeepBytesAlive({ 11, 12 }), [&done]()
        { done.callback(); });

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, ClearNoItem)
{
    AddItem(KeepBytesAlive({ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    iterator.ErasePrevious([]() {});
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, AddPartialItem)
{
    AddPartialItem(KeepBytesAlive({ 11 }), 3);
    AddItem(KeepBytesAlive({ 12 }));
    AddItem(KeepBytesAlive({ 13 }));

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 3, 0, 11, 12, 13, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, AddSecondItem)
{
    AddItem(KeepBytesAlive({ 11 }));
    AddItem(KeepBytesAlive({ 12 }));

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 1, 0, 11, 0xf8, 1, 0, 12, 0xff }), flash.sectors[0]);
}

TEST_F(CyclicStoreTest, FillSector)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
                  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, AddItemInNewSector)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
                  { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, FillUnusedSpaceWhenAddingItem)
{
    AddItem(KeepBytesAlive({ 11, 12, 13 }));
    AddItem(KeepBytesAlive({ 21 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 3, 0, 11, 12, 13, 0x7f, 0xff, 0xff },
                  { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, FillOneByteOfUnusedSpaceWhenAddingItem)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15 }));
    AddItem(KeepBytesAlive({ 21 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  {
                      0xfc,
                      0xf8,
                      5,
                      0,
                      11,
                      12,
                      13,
                      14,
                      15,
                      0x7f,
                  },
                  { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, AddFirstItemWhenFlashStopsAfterNSteps)
{
    std::vector<std::vector<uint8_t>> flashAfterNSteps;

    for (uint8_t i = 1; i != 7; ++i)
    {
        cyclicStore.Clear(infra::emptyFunction);
        ExecuteAllActions();
        flash.stopAfterWriteSteps = i;

        AddItem(KeepBytesAlive({ 11, 12 }));

        flashAfterNSteps.push_back(flash.sectors[0]);
    }

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                  { 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                  { 0xfc, 0xfe, 2, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                  { 0xfc, 0xfc, 2, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                  { 0xfc, 0xfc, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff },
                  { 0xfc, 0xf8, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff } }),
        flashAfterNSteps);
}

TEST_F(CyclicStoreTest, AddItemWhichCausesSectorToBeErased)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));
    AddItem(KeepBytesAlive({ 31, 32, 33, 34, 35, 36 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
                  { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, AddItemOverflowingSectorWhichCausesSectorToBeErased)
{
    flash.sectors[0].resize(14);
    flash.sectors[1].resize(14);

    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));
    AddItem(KeepBytesAlive({ 31, 32, 33, 34, 35, 36 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36, 0xff, 0xff, 0xff, 0xff },
                  { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26, 0x7f, 0xff, 0xff, 0xff },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, ReadFirstItem)
{
    AddItem(KeepBytesAlive({ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ClearFirstItem)
{
    AddItem(KeepBytesAlive({ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    iterator.ErasePrevious([]() {});
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf0, 2, 0, 11, 12, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{}), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadFirstItemTwice)
{
    // build
    AddItem(KeepBytesAlive({ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12 }), Read(iterator));

    // operate/check
    iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadItemWithInsufficientBufferSpace)
{
    AddItem(KeepBytesAlive({ 11, 12 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 11 }), Read(iterator, 1));
}

TEST_F(CyclicStoreTest, ReadSecondItem)
{
    AddItem(KeepBytesAlive({ 11 }));
    AddItem(KeepBytesAlive({ 21 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{ 21 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadSecondItemInNewSector)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
                  { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{ 21 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadSecondItemAfterSkipSpace)
{
    AddItem(KeepBytesAlive({ 11, 12, 13 }));
    AddItem(KeepBytesAlive({ 21 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 3, 0, 11, 12, 13, 0x7f, 0xff, 0xff },
                  { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{ 21 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadSecondItemAfterSkipSpaceOfOneByte)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15 }));
    AddItem(KeepBytesAlive({ 21 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  {
                      0xfc,
                      0xf8,
                      5,
                      0,
                      11,
                      12,
                      13,
                      14,
                      15,
                      0x7f,
                  },
                  { 0xfe, 0xf8, 1, 0, 21, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{ 21 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadSecondItemAfterSkipSpaceReallySkipsSector)
{
    flash.sectors = std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 1, 0, 11, 0x7f, 0xf8, 1, 0, 21 },
        { 0xfe, 0xf8, 1, 0, 31 },
    };

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{ 31 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadInEmptyYieldsEmptyRange)
{
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{}), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadPastEndYieldsEmptyRange)
{
    AddItem(KeepBytesAlive({ 11, 12, 13 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{}), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadPastEndInAFullFlashYieldsEmptyRange)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);
    EXPECT_EQ((std::vector<uint8_t>{}), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadWhileAddWaitsForAddToFinish)
{
    std::vector<uint8_t> data = { 11, 12 };
    cyclicStore.Add(data, infra::emptyFunction);

    infra::MockCallback<void(std::vector<uint8_t>)> mock;

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(2, 0);
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result)
        { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{ 11, 12 }));
    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, AddWhileClearWaitsforClearToFinish)
{
    AddItem(KeepBytesAlive({ 11, 12 }));

    infra::MockCallback<void()> done;
    flash.delaySignalEraseDone = true;
    cyclicStore.Clear([&done]()
        { done.callback(); });
    infra::Function<void()> onEraseDone = flash.onEraseDone;
    ExecuteAllActions();
    flash.delaySignalEraseDone = false;

    AddItem(KeepBytesAlive({ 21, 22 }));

    EXPECT_CALL(done, callback());
    onEraseDone();

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 21, 22 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ReadItemAfterCyclicalLogging)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));
    AddItem(KeepBytesAlive({ 31, 32, 33, 34, 35, 36 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
                  { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }), Read(iterator));
}

TEST_F(CyclicStoreTest, ClearFirstItemAfterWrapAroundDoesNothing)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);

    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));
    AddItem(KeepBytesAlive({ 31, 32, 33, 34, 35, 36 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
                  { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);

    iterator.ErasePrevious([]() {});
    ExecuteAllActions();

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
                  { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, ReadItemAfterCyclicalLogging2)
{
    std::vector<std::vector<uint8_t>> sectors = { { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
        { 0xfe, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 } };
    flash.sectors = sectors;
    ReConstructCyclicStore();

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12, 13, 14, 15, 16 }), Read(iterator));

    AddItem(KeepBytesAlive({ 31, 32, 33, 34, 35, 36 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfe, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36 },
                  { 0xfc, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 } }),
        flash.sectors);

    EXPECT_EQ((std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }), Read(iterator));
}

TEST_F(CyclicStoreTest, RecoverAndReadFirstItem)
{
    ReConstructCyclicStore();
    AddItem(KeepBytesAlive({ 11, 12 }));

    ReConstructCyclicStore();
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12 }), Read(iterator));
}

TEST_F(CyclicStoreTest, RecoverFromAllDifferentGoodScenarios)
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
    const std::vector<uint8_t> usedWithSpaceLeft = { 0xfe, 0xf8, 1, 0, 44, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    const std::vector<uint8_t> usedWithSpaceLeftWritten = { 0xfe, 0xf8, 1, 0, 44, 0xf8, 1, 0, 66, 0xff, 0xff, 0xff, 0xff, 0xff };
    const std::vector<uint8_t> written = { 0xfe, 0xf8, 1, 0, 66 };
    const std::vector<uint8_t> usedAsFirst = { 0xfc, 0xf8, 1, 0, 44 };

    const std::array<Scenario, 11> scenarios = { { { std::vector<uint8_t>{ 55 },
                                                       std::vector<std::vector<uint8_t>>{ first, empty },
                                                       std::vector<std::vector<uint8_t>>{ first, written } },
        { std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, used, empty },
            std::vector<std::vector<uint8_t>>{ first, used, written } },
        { std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, used, empty, empty },
            std::vector<std::vector<uint8_t>>{ first, used, written, empty } },
        { std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ first, used, used },
            std::vector<std::vector<uint8_t>>{ written, usedAsFirst, used } },
        { std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ used, empty, used },
            std::vector<std::vector<uint8_t>>{ used, written, usedAsFirst } },
        { std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ empty, used, used },
            std::vector<std::vector<uint8_t>>{ written, usedAsFirst, used } },
        { std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ used, empty },
            std::vector<std::vector<uint8_t>>{ usedAsFirst, written } },
        { std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ used, first, used },
            std::vector<std::vector<uint8_t>>{ used, written, usedAsFirst } },
        { std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ used, used, first },
            std::vector<std::vector<uint8_t>>{ usedAsFirst, used, written } },
        { std::vector<uint8_t>{ 44 },
            std::vector<std::vector<uint8_t>>{ used, used, empty, used, used },
            std::vector<std::vector<uint8_t>>{ used, used, written, usedAsFirst, used } },
        { std::vector<uint8_t>{ 55 },
            std::vector<std::vector<uint8_t>>{ used, usedWithSpaceLeft, first },
            std::vector<std::vector<uint8_t>>{ used, usedWithSpaceLeftWritten, first } } } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors = scenarios[index].flashContents;

        ReConstructCyclicStore();

        services::CyclicStore::Iterator iterator = cyclicStore.Begin();
        EXPECT_EQ(scenarios[index].expected, Read(iterator));

        AddItem(KeepBytesAlive({ 66 }));

        EXPECT_EQ(scenarios[index].flashContentsAfterAddition, flash.sectors);
    }
}

TEST_F(CyclicStoreTest, RecoverAfterRemovedItem)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
                  { 0xfe, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);

    iterator.ErasePrevious([]() {});
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf0, 6, 0, 11, 12, 13, 14, 15, 16 },
                  { 0xfe, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);

    ReConstructCyclicStore();

    iterator = cyclicStore.Begin();
    EXPECT_EQ((std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }), Read(iterator));
}

TEST_F(CyclicStoreTest, WriteAfterRecoverAfterRemovedItem)
{
    infra::ReConstruct(flash, 2, 30);
    ReConstructCyclicStore();

    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    Read(iterator);

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);

    iterator.ErasePrevious([]() {});
    ExecuteAllActions();

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf0, 6, 0, 11, 12, 13, 14, 15, 16, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);

    ReConstructCyclicStore();

    AddItem(KeepBytesAlive({ 31, 32, 33, 34, 35, 36 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf0, 6, 0, 11, 12, 13, 14, 15, 16, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26, 0xf8, 6, 0, 31, 32, 33, 34, 35, 36, 0xff, 0xff },
                  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
              }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, InAllCorruptScenariosFlashIsErased)
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

        ReConstructCyclicStore();

        std::vector<std::vector<uint8_t>> emptyFlash = flash.sectors;
        for (auto& sector : emptyFlash)
            sector = std::vector<uint8_t>(sector.size(), 0xff);

        EXPECT_EQ(emptyFlash, flash.sectors);
    }
}

TEST_F(CyclicStoreTest, AfterRecoveryItemsAreInsertedAfterCurrentContents)
{
    flash.sectors = std::vector<std::vector<uint8_t>>{
        { 0xfc, 0xf8, 1, 0, 16, 0xff, 0xff, 0xff, 0xff },
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    };

    ReConstructCyclicStore();
    AddItem(KeepBytesAlive({ 32 }));

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 1, 0, 16, 0xf8, 1, 0, 32 },
                  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }),
        flash.sectors);
}

TEST_F(CyclicStoreTest, RecoveryAfterPartialWriteContinuesAtTheCorrectPosition)
{
    struct Scenario
    {
        std::vector<uint8_t> flashContents;
        std::vector<uint8_t> flashContentsAfterAdd;
    };

    const std::array<Scenario, 5> scenarios = { { {
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
        } } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index].flashContents;

        ReConstructCyclicStore();
        AddItem(KeepBytesAlive({ 22 }));

        EXPECT_EQ(scenarios[index].flashContentsAfterAdd, flash.sectors[0]);
    }
}

TEST_F(CyclicStoreTest, RecoveryAfterCorruptionContinuesAtTheNextSector)
{
    const std::array<std::vector<uint8_t>, 3> scenarios = { { { 0xfc, 0xaa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xf8, 0xaa, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0xfc, 0xfc, 2, 0, 11, 12, 0xaa, 0xff, 0xff, 0xff } } };

    const std::vector<uint8_t> nextSector = { 0xfe, 0xf8, 1, 0, 22, 0xff, 0xff, 0xff, 0xff, 0xff };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index];
        flash.sectors[1] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

        ReConstructCyclicStore();
        AddItem(KeepBytesAlive({ 22 }));

        EXPECT_EQ(nextSector, flash.sectors[1]);
    }
}

TEST_F(CyclicStoreTest, ReadSkipsIncompleteData)
{
    const std::array<std::vector<uint8_t>, 4> scenarios = { {
        { 0xfc, 0xfe, 0xff, 0xff, 0xf8, 1, 0, 22, 0xff, 0xff },
        { 0xfc, 0xfe, 2, 0, 0xf8, 1, 0, 22, 0xff, 0xff },
        { 0xfc, 0xfc, 2, 0, 0xff, 0xff, 0xf8, 1, 0, 22 },
        { 0xfc, 0xfc, 2, 0, 11, 12, 0xf8, 1, 0, 22 },
    } };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index];

        ReConstructCyclicStore();

        services::CyclicStore::Iterator iterator = cyclicStore.Begin();
        EXPECT_EQ((std::vector<uint8_t>{ 22 }), Read(iterator));
    }
}

TEST_F(CyclicStoreTest, OnCorruptDataReadSkipsToNextSector)
{
    const std::array<std::vector<uint8_t>, 3> scenarios = { { { 0xfc, 0xaa, 0xf8, 1, 0, 22 },
        { 0xfc, 0xf8, 0xaa, 0, 0xf8, 1, 0, 22 },
        { 0xfc, 0xfc, 2, 0, 11, 12, 0xaa, 0xf8, 1, 0, 22 } } };

    flash.sectors[1] = { 0xfe, 0xf8, 1, 0, 33 };

    for (std::size_t index = 0; index != scenarios.size(); ++index)
    {
        flash.sectors[0] = scenarios[index];

        ReConstructCyclicStore();

        services::CyclicStore::Iterator iterator = cyclicStore.Begin();
        EXPECT_EQ((std::vector<uint8_t>{ 33 }), Read(iterator));
    }
}

TEST_F(CyclicStoreTest, WhenSectorIsErasedIteratorIsMovedForward)
{
    AddItem(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }));
    AddItem(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }));

    ASSERT_EQ((std::vector<std::vector<uint8_t>>{
                  { 0xfc, 0xf8, 6, 0, 11, 12, 13, 14, 15, 16 },
                  { 0xfe, 0xf8, 6, 0, 21, 22, 23, 24, 25, 26 },
              }),
        flash.sectors);

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();

    std::vector<uint8_t> erasingSectorData = { 31 };
    cyclicStore.Add(erasingSectorData, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }), Read(iterator));
}

TEST_F(CyclicStoreTest, TestParallelAddAndClear)
{
    infra::MockCallback<void(std::vector<uint8_t>)> mock;
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();
    std::vector<uint8_t> readDataBuffer(6, 0);

    EXPECT_CALL(mock, callback(std::vector<uint8_t>{}));
    iterator.Read(readDataBuffer, [&mock](infra::ByteRange result)
        { mock.callback(std::vector<uint8_t>(result.begin(), result.end())); });

    cyclicStore.Add(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }), infra::emptyFunction);
    cyclicStore.Clear(infra::emptyFunction);

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, TestParallelAddPartialAndClear)
{
    cyclicStore.AddPartial(KeepBytesAlive({ 11 }), 3, infra::emptyFunction);
    cyclicStore.Clear(infra::emptyFunction);
    ExecuteAllActions();

    AddItem(KeepBytesAlive({ 12 }));
    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xfc, 3, 0, 11, 12, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    AddItem(KeepBytesAlive({ 13 }));
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, TestParallelAddAndClearUrgent)
{
    AddItem(KeepBytesAlive({ 1 }));

    cyclicStore.Add(KeepBytesAlive({ 12 }), infra::emptyFunction);
    cyclicStore.ClearUrgent(infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0xfc, 0xf8, 1, 0, 12, 0xff, 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);

    ExecuteAllActions();
}

TEST_F(CyclicStoreTest, when_iterator_is_at_start_of_an_empty_sector_newly_added_item_is_available_in_next_read)
{
    services::CyclicStore::Iterator iterator = cyclicStore.Begin();

    cyclicStore.Add(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }), infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12, 13, 14, 15, 16 }), Read(iterator));

    cyclicStore.Add(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }), infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }), Read(iterator));
}

TEST_F(CyclicStoreTest, when_iterator_is_at_the_end_of_flash_newly_added_item_is_available_in_next_read)
{
    ReConstructFlashAndCyclicStore(2, 30);

    services::CyclicStore::Iterator iterator = cyclicStore.Begin();

    cyclicStore.Add(KeepBytesAlive({ 11, 12, 13, 14, 15, 16 }), infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 11, 12, 13, 14, 15, 16 }), Read(iterator));

    EXPECT_EQ((std::vector<uint8_t>{}), Read(iterator));

    cyclicStore.Add(KeepBytesAlive({ 21, 22, 23, 24, 25, 26 }), infra::emptyFunction);
    EXPECT_EQ((std::vector<uint8_t>{ 21, 22, 23, 24, 25, 26 }), Read(iterator));
}
