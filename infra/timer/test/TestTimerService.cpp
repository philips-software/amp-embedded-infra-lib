#include "infra/timer/TimerService.hpp"
#include "gtest/gtest.h"

namespace
{
    class MinimalTimerService
        : public infra::TimerService
    {
    public:
        explicit MinimalTimerService(uint32_t id)
            : infra::TimerService(id)
        {}

        infra::TimePoint Now() const override
        {
            return {};
        }

        infra::Duration Resolution() const override
        {
            return {};
        }
    };
}

#ifndef EMIL_MUTATION_TESTING

TEST(TimerServiceDeathTest, constructing_already_registered_service_aborts)
{
    MinimalTimerService service(1);
    EXPECT_DEATH({ MinimalTimerService otherService(1); (void)otherService; }, "");
}

TEST(TimerServiceDeathTest, get_timer_service_with_unknown_id_aborts)
{
    EXPECT_DEATH(infra::TimerService::GetTimerService(99), "");
}

#endif
