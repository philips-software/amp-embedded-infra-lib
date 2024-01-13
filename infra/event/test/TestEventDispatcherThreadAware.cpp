#include "infra/event/EventDispatcherThreadAware.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <thread>

class EventDispatcherThreadAwareIdleCounting
    : public infra::EventDispatcherThreadAware::WithSize<50>
{
protected:
    void Idle() override
    {
        ++idleCount;
        infra::EventDispatcherThreadAware::WithSize<50>::Idle();
    }

public:
    int idleCount = 0;
};

class EventDispatcherThreadAwareTest
    : public testing::Test
    , public EventDispatcherThreadAwareIdleCounting
{};

TEST_F(EventDispatcherThreadAwareTest, scheduled_action_is_executed)
{
    bool done = false;

    std::thread t([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            infra::EventDispatcher::Instance().Schedule([&]()
                {
                    done = true;
                });
        });

    ExecuteUntil([&]()
        {
            return done;
        });

    EXPECT_THAT(idleCount, testing::Eq(1));

    t.join();
}
