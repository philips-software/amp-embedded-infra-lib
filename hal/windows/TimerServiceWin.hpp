#ifndef HAL_TIMER_SERVICE_WIN_HPP
#define HAL_TIMER_SERVICE_WIN_HPP

#include "infra/timer/TimerService.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>

namespace hal
{
    class TimerServiceWin
        : public infra::TimerService
    {
    public:
        TimerServiceWin(uint32_t id = infra::systemTimerServiceId);
        ~TimerServiceWin();

        virtual void NextTriggerChanged() override;
        virtual infra::TimePoint Now() const override;
        virtual infra::Duration Resolution() const override;

    private:
        void WaitForTrigger();

    private:
        std::condition_variable_any condition;
        std::recursive_mutex mutex;
        infra::TimePoint nextTrigger;
        bool quit = false;
        std::thread triggerThread;
    };
}

#endif
