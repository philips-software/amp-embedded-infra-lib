#include "hal/bsd/TimerServiceBsd.hpp"
#include "infra/event/EventDispatcher.hpp"
#include <cassert>

namespace hal
{
    TimerServiceBsd::TimerServiceBsd(uint32_t id)
        : TimerService(id)
        , triggerThread([this]() { WaitForTrigger(); })
    {
        nextTrigger = NextTrigger();
    }

    TimerServiceBsd::~TimerServiceBsd()
    {
        quit = true;
        condition.notify_one();
        triggerThread.join();
    }

    void TimerServiceBsd::NextTriggerChanged()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        nextTrigger = NextTrigger();
        condition.notify_one();
    }

    infra::TimePoint TimerServiceBsd::Now() const
    {
        return std::chrono::system_clock::now();
    }

    infra::Duration TimerServiceBsd::Resolution() const
    {
        return std::chrono::duration<long long, std::chrono::system_clock::period>(1);
    }

    void TimerServiceBsd::WaitForTrigger()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);

        while (!quit)
        {
            if (nextTrigger != infra::TimePoint::max())
                condition.wait_until(lock, nextTrigger);
            else
                condition.wait(lock);

            if (nextTrigger != infra::TimePoint::max() && nextTrigger <= Now())
            {
                nextTrigger = infra::TimePoint::max();

                infra::EventDispatcher::Instance().Schedule([this]()
                {
                    std::unique_lock<std::recursive_mutex> lock(mutex);
                    Progressed(Now());
                    NextTriggerChanged();
                });
            }
        }
    }
}
