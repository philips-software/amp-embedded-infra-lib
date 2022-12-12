#ifndef INFRA_CLOCK_FIXTURE_HPP
#define INFRA_CLOCK_FIXTURE_HPP

#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/timer/TimerService.hpp"
#include "infra/timer/test_helper/PerfectTimerService.hpp"
#include "gmock/gmock.h"

namespace infra
{
    class ClockFixture
        : public EventDispatcherWithWeakPtrFixture
    {
    public:
        explicit ClockFixture(uint32_t timerSericeId = systemTimerServiceId);
        ClockFixture(const ClockFixture&) = delete;
        ClockFixture& operator=(const ClockFixture&) = delete;
        ~ClockFixture();

        // Forward the 'apparent' system time, thereby triggering any timers that were waiting
        void ForwardTime(Duration time);
        // Forward the system time with a jump
        void JumpForwardTime(Duration time);

        class TimeMatcherHelper;

        TimeMatcherHelper After(infra::Duration duration) const;

        static std::string TimeToString(TimePoint time);

        PerfectTimerService systemTimerService;
    };

    class ClockFixture::TimeMatcherHelper
    {
    public:
        explicit TimeMatcherHelper(infra::TimePoint expectedCallTime);

        template<class... Args>
        operator testing::Matcher<testing::tuple<Args...>>() const
        {
            return testing::MakeMatcher(new TimeMatcher<Args...>(expectedCallTime));
        }

        template<class... Args>
        operator testing::Matcher<const testing::tuple<Args...>&>() const
        {
            return testing::MakeMatcher(new TimeMatcher<Args...>(expectedCallTime));
        }

        template<class... Args>
        class TimeMatcher
            : public testing::MatcherInterface<const std::tuple<Args...>&>
        {
        public:
            explicit TimeMatcher(infra::TimePoint expectedCallTime);

            virtual bool MatchAndExplain(const std::tuple<Args...>& x, testing::MatchResultListener* listener) const override;
            virtual void DescribeTo(std::ostream* os) const override;

        private:
            infra::TimePoint expectedCallTime;
        };

    private:
        infra::TimePoint expectedCallTime;
    };

    ////    Implementation    ////

    template<class... Args>
    ClockFixture::TimeMatcherHelper::TimeMatcher<Args...>::TimeMatcher(infra::TimePoint expectedCallTime)
        : expectedCallTime(expectedCallTime)
    {}

    template<class... Args>
    bool ClockFixture::TimeMatcherHelper::TimeMatcher<Args...>::MatchAndExplain(const std::tuple<Args...>& x, testing::MatchResultListener* listener) const
    {
        return expectedCallTime == infra::Now();
    }

    template<class... Args>
    void ClockFixture::TimeMatcherHelper::TimeMatcher<Args...>::DescribeTo(std::ostream* os) const
    {
        if (os)
        {
            *os << "To be called at " << TimeToString(expectedCallTime) << std::endl
                << "                   It is now       " << TimeToString(infra::Now()) << std::endl;
        }
    }

}

#endif
