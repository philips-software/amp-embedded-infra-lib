#include "freertos_std_thread/stdcondition_variable.hpp"
#include "gtest/gtest.h"
#include <memory>

class SemaphoreFixture
    : public testing::Test
{
public:
    SemaphoreFixture()
        : p1(&p1)
        , p2(&p2)
    {}

public:
    SemaphoreMock mock;
    std::condition_variable condition_variable;
    std::mutex mutex;
    void* p1;
    void* p2;
};

class SemaphoreWaitFixture
    : public SemaphoreFixture
{
public:
    SemaphoreWaitFixture()
    {
        EXPECT_CALL(mock, SemaphoreCreateBinary())
            .WillOnce(testing::Return(p1));
        EXPECT_CALL(mock, SemaphoreDelete(p1))
            .Times(1);

        inSequence.reset(new testing::InSequence());
        EXPECT_CALL(mock, SemaphoreTake(p1, 0))
            .WillOnce(testing::Return(true));
    }

    std::unique_ptr<testing::InSequence> inSequence;
};

TEST_F(SemaphoreFixture, TestCreation)
{}

TEST_F(SemaphoreFixture, WhenNoOneIsWaitingNotifyOneDoesNothing)
{
    // No expectations, no one is waiting on the variable
    condition_variable.notify_one();
}

TEST_F(SemaphoreFixture, WhenNoOneIsWaitingNotifyAllDoesNothing)
{
    condition_variable.notify_all();
}

TEST_F(SemaphoreWaitFixture, Wait)
{
    EXPECT_CALL(mock, SemaphoreTake(p1, portMAX_DELAY))
        .WillOnce(testing::Return(true));

    std::unique_lock<std::mutex> lock(mutex);

    condition_variable.wait(lock);
}

TEST_F(SemaphoreWaitFixture, WhenSemaphoreTakeSucceedsWaitForReturnsNoTimeout)
{
    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Return(true));

    std::unique_lock<std::mutex> lock(mutex);

    EXPECT_EQ(std::cv_status::no_timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}

TEST_F(SemaphoreWaitFixture, WhenSemaphoreTakeFailsWaitForTriesAgainAndReturnsNoTimeout)
{
    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Return(false));
    EXPECT_CALL(mock, SemaphoreTake(p1, 0))
        .WillOnce(testing::Return(true));

    std::unique_lock<std::mutex> lock(mutex);

    EXPECT_EQ(std::cv_status::no_timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}

TEST_F(SemaphoreWaitFixture, WhenSemaphoreTakeFailsWaitForTriesAgainAndReturnsTimeout)
{
    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Return(false));
    EXPECT_CALL(mock, SemaphoreTake(p1, 0))
        .WillOnce(testing::Return(false));

    std::unique_lock<std::mutex> lock(mutex);

    EXPECT_EQ(std::cv_status::timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}

TEST_F(SemaphoreWaitFixture, WhenWaitingNotifyOneReleasesSemaphore)
{
    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Invoke([this](xSemaphoreHandle, portTickType)
            {
                condition_variable.notify_one();
                return true;
            }));

    EXPECT_CALL(mock, SemaphoreGive(p1));

    std::unique_lock<std::mutex> lock(mutex);

    EXPECT_EQ(std::cv_status::no_timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}

TEST_F(SemaphoreWaitFixture, WhenWaitingNotifyAllReleasesSemaphore)
{
    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Invoke([this](xSemaphoreHandle, portTickType)
            {
                condition_variable.notify_all();
                return true;
            }));

    EXPECT_CALL(mock, SemaphoreGive(p1));

    std::unique_lock<std::mutex> lock(mutex);

    EXPECT_EQ(std::cv_status::no_timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}

TEST_F(SemaphoreFixture, WhenWaitingTwiceNotifyAllReleasesTwoSemaphores)
{
    testing::InSequence s;

    EXPECT_CALL(mock, SemaphoreCreateBinary())
        .WillOnce(testing::Return(p1));
    EXPECT_CALL(mock, SemaphoreTake(p1, 0))
        .WillOnce(testing::Return(true));

    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Invoke([this](xSemaphoreHandle, portTickType)
            {
                std::unique_lock<std::mutex> lock(mutex);
                condition_variable.wait_for(lock, std::chrono::seconds(2));
                return true;
            }));

    EXPECT_CALL(mock, SemaphoreCreateBinary())
        .WillOnce(testing::Return(p2));
    EXPECT_CALL(mock, SemaphoreTake(p2, 0))
        .Times(1);

    EXPECT_CALL(mock, SemaphoreTake(p2, 2000))
        .WillOnce(testing::Invoke([this](xSemaphoreHandle, portTickType)
            {
                condition_variable.notify_all();
                return true;
            }));

    EXPECT_CALL(mock, SemaphoreGive(p1));
    EXPECT_CALL(mock, SemaphoreGive(p2));

    EXPECT_CALL(mock, SemaphoreDelete(p2));
    EXPECT_CALL(mock, SemaphoreDelete(p1));

    std::unique_lock<std::mutex> lock(mutex);
    EXPECT_EQ(std::cv_status::no_timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}

TEST_F(SemaphoreFixture, WhenWaitingTwiceNotifyOneReleasesFirstSemaphore)
{
    testing::InSequence s;

    EXPECT_CALL(mock, SemaphoreCreateBinary())
        .WillOnce(testing::Return(p1));
    EXPECT_CALL(mock, SemaphoreTake(p1, 0))
        .WillOnce(testing::Return(true));

    EXPECT_CALL(mock, SemaphoreTake(p1, 3000))
        .WillOnce(testing::Invoke([this](xSemaphoreHandle, portTickType)
            {
                std::unique_lock<std::mutex> lock(mutex);
                condition_variable.wait_for(lock, std::chrono::seconds(2));
                return true;
            }));

    EXPECT_CALL(mock, SemaphoreCreateBinary())
        .WillOnce(testing::Return(p2));
    EXPECT_CALL(mock, SemaphoreTake(p2, 0))
        .Times(1);

    EXPECT_CALL(mock, SemaphoreTake(p2, 2000))
        .WillOnce(testing::Invoke([this](xSemaphoreHandle, portTickType)
            {
                condition_variable.notify_one();
                return false;
            })); // Second semaphore times out

    EXPECT_CALL(mock, SemaphoreGive(p1));

    EXPECT_CALL(mock, SemaphoreTake(p2, 0)) // As a result of the timeout, it is tried again
        .WillOnce(testing::Return(false));

    EXPECT_CALL(mock, SemaphoreDelete(p2));
    EXPECT_CALL(mock, SemaphoreDelete(p1));

    std::unique_lock<std::mutex> lock(mutex);
    EXPECT_EQ(std::cv_status::no_timeout, condition_variable.wait_for(lock, std::chrono::seconds(3)));
}
