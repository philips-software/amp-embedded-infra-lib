#ifndef OSAL_THREADX_STD_THREAD_ONCE_HPP
#define OSAL_THREADX_STD_THREAD_ONCE_HPP

#include "stdmutex.hpp"
#include "stdcondition_variable.hpp"
#include <atomic>
#include <utility>

namespace std
{
    struct once_flag
    {
    public:
        once_flag() noexcept = default;
        once_flag(const once_flag&) = delete;
        once_flag& operator=(const once_flag&) = delete;

        template<typename _Callable, typename... _Args>
            friend void call_once(once_flag& __once, _Callable&& __f, _Args&&... __args);

    private:
        atomic_bool started = ATOMIC_VAR_INIT(false);
        atomic_bool done = ATOMIC_VAR_INIT(false);
        mutex mut;
        condition_variable condition;
    };

    template<typename Callable, typename... Args>
    void call_once(once_flag& once, Callable&& f, Args&&... args)
    {
        if (once.done)
            return;

        bool expected = true;
        if (once.started.compare_exchange_strong(expected, true))
        {
            unique_lock<mutex> lock(once.mut);
            f(forward<Args>(args)...);
            once.done = true;
            once.condition.notify_all();
        }
        else
        {
            unique_lock<mutex> lock(once.mut);
            while (!once.done)
                once.condition.wait(lock);
        }
    }
}

#endif
