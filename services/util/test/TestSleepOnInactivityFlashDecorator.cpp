#include "hal/interfaces/test_doubles/FlashMock.hpp"
#include "hal/interfaces/test_doubles/SleepableMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/SleepOnInactivityFlashDecorator.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <chrono>

namespace services
{
    template<typename T>
    class SleepOnInactivityFlashDecoratorTest
        : public testing::Test
        , public infra::ClockFixture
    {
    public:
        SleepOnInactivityFlashDecoratorTest()
        {
            ON_CALL(flash, WriteBuffer(testing::_, testing::_, testing::_)).WillByDefault(testing::InvokeArgument<2>());
            ON_CALL(flash, ReadBuffer(testing::_, testing::_, testing::_)).WillByDefault(testing::InvokeArgument<2>());
            ON_CALL(flash, EraseSectors(testing::_, testing::_, testing::_)).WillByDefault(testing::InvokeArgument<2>());

            ON_CALL(sleepable, Sleep(testing::_)).WillByDefault(testing::InvokeArgument<0>());
            ON_CALL(sleepable, Wake(testing::_)).WillByDefault(testing::InvokeArgument<0>());

            EXPECT_CALL(sleepable, Sleep(testing::_));
            Timeout();
            testing::Mock::VerifyAndClearExpectations(&sleepable);
        }

        void Timeout()
        {
            ForwardTime(std::chrono::milliseconds(timeoutMs + 1));
        }

        static constexpr int timeoutMs = 5;
        testing::StrictMock<hal::CleanFlashMockBase<T>> flash;
        testing::StrictMock<hal::SleepableMock> sleepable;
        SleepOnInactivityFlashDecoratorBase<T> decorator{ flash, sleepable, std::chrono::milliseconds(timeoutMs) };

        std::array<uint8_t, 32> dummyArray;
    };

    typedef testing::Types<uint32_t, uint64_t> FlashTypes;
    TYPED_TEST_SUITE(SleepOnInactivityFlashDecoratorTest, FlashTypes);

    TYPED_TEST(SleepOnInactivityFlashDecoratorTest, WriteBufferDoesWakeWriteSleep)
    {
        uint32_t address = 42;
        auto buffer = infra::MakeConstByteRange(this->dummyArray);

        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, WriteBuffer(buffer, address, testing::_));
        infra::VerifyingFunction<void()> onDone;
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        this->decorator.WriteBuffer(buffer, address, onDone);
        this->Timeout();
    }

    TYPED_TEST(SleepOnInactivityFlashDecoratorTest, ReadBufferDoesWakeReadSleep)
    {
        uint32_t address = 42;
        auto buffer = infra::MakeByteRange(this->dummyArray);

        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, ReadBuffer(buffer, address, testing::_));
        infra::VerifyingFunction<void()> onDone;
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        this->decorator.ReadBuffer(buffer, address, onDone);
        this->Timeout();
    }

    TYPED_TEST(SleepOnInactivityFlashDecoratorTest, EraseSectorsDoesWakeEraseSleep)
    {
        uint32_t beginIndex = 2;
        uint32_t endIndex = 5;

        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, EraseSectors(beginIndex, endIndex, testing::_));
        infra::VerifyingFunction<void()> onDone;
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        this->decorator.EraseSectors(beginIndex, endIndex, onDone);
        this->Timeout();
    }

    TYPED_TEST(SleepOnInactivityFlashDecoratorTest, NoSleepBetweenConsecutiveOperations)
    {
        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, EraseSectors(1, 2, testing::_));
        infra::VerifyingFunction<void()> onDone;
        EXPECT_CALL(this->flash, EraseSectors(3, 4, testing::_));
        infra::VerifyingFunction<void()> onDone2;
        EXPECT_CALL(this->sleepable, Sleep(testing::_));

        this->decorator.EraseSectors(1, 2, onDone);
        this->decorator.EraseSectors(3, 4, onDone2);
        this->Timeout();
    }

    TYPED_TEST(SleepOnInactivityFlashDecoratorTest, NoTimeoutNoSleep)
    {
        testing::InSequence inSequenceEnabler;
        EXPECT_CALL(this->sleepable, Wake(testing::_));
        EXPECT_CALL(this->flash, EraseSectors(testing::_, testing::_, testing::_));

        this->decorator.EraseSectors(1, 2, infra::emptyFunction);
    }
}
