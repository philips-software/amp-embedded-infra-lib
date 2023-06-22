#include "hal/generic/TimerServiceGeneric.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace hal
{
    TimerServiceGeneric::TimerServiceGeneric(uint32_t id)
        : infra::TimerService(id)
        , nextTrigger(NextTrigger())
        , triggerThread([this]()
              {
                  WaitForTrigger();
              })
    {}

    TimerServiceGeneric::~TimerServiceGeneric()
    {
        quit = true;
        condition.notify_one();
        triggerThread.join();
    }

    void TimerServiceGeneric::NextTriggerChanged()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        nextTrigger = NextTrigger();
        condition.notify_one();
    }

    infra::TimePoint TimerServiceGeneric::Now() const
    {
        return std::chrono::system_clock::now();
    }

    infra::Duration TimerServiceGeneric::Resolution() const
    {
        return std::chrono::duration<long long, std::chrono::system_clock::period>(1);
    }

    void TimerServiceGeneric::WaitForTrigger()
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
                        std::unique_lock<std::recursive_mutex> eventLock(mutex);
                        Progressed(Now());
                        NextTriggerChanged();
                    });
            }
        }
    }
}
