#ifndef OSAL_FREERTOS_STD_THREAD_CONDITION_VARIABLE_HPP
#define OSAL_FREERTOS_STD_THREAD_CONDITION_VARIABLE_HPP

#include <chrono>
#ifndef _MSC_VER
#include "stdmutex.hpp"
#else
#include <mutex>
#endif
#include "FreeRTOS.h"
#include "semphr.h"

namespace std
{
    enum class cv_status { no_timeout, timeout };

    template<typename Lock>
    class Unlock
    {
    public:
        explicit Unlock(Lock& lock)
            : mLock(lock)
        {
            lock.unlock();
        }

        ~Unlock() noexcept
        {
            mLock.lock();
        }

    private:
        Lock& mLock;

        Unlock(const Unlock&);
        Unlock& operator=(const Unlock&);
    };

    class condition_variable
    {
    private:
        typedef chrono::system_clock clock_type;

        struct WaitingEntry
        {
            xSemaphoreHandle semaphore;
            WaitingEntry* next;
            WaitingEntry* previous;
        };

    public:
        typedef WaitingEntry* native_handle_type;

        constexpr condition_variable() noexcept
            : mWaitingFront(nullptr)
            , mWaitingBack(nullptr)
        {}

        ~condition_variable() noexcept = default;

        void notify_one() noexcept
        {
            lock_guard<mutex> lock(mMutex);

            if (mWaitingBack != nullptr)
            {
                xSemaphoreGive(mWaitingBack->semaphore);
                popBack();
            }
        }

        void notify_all() noexcept
        {
            lock_guard<mutex> lock(mMutex);

            while (mWaitingBack != nullptr)
            {
                xSemaphoreGive(mWaitingBack->semaphore);
                popBack();
            }
        }

        void wait(unique_lock<mutex>& lock)
        {
            wait_for_impl(lock, portMAX_DELAY);
        }

        template<typename Predicate>
        void wait(unique_lock<mutex>& lock, Predicate p)
        {
            while (!p())
                wait(lock);
        }

        template<typename Duration>
        cv_status wait_until(unique_lock<mutex>& lock, const chrono::time_point<clock_type, Duration>& time_point)
        {
            return wait_for(lock, time_point - clock_type::now());
        }

        template<typename Clock, typename Duration>
        cv_status wait_until(unique_lock<mutex>& lock, const chrono::time_point<Clock, Duration>& time_point)
        {
            return wait_for(lock, time_point - Clock::now());
        }

        template<typename Clock, typename Duration, typename Predicate>
        bool wait_until(unique_lock<mutex>& lock, const chrono::time_point<Clock, Duration>& time_point, Predicate p)
        {
            while (!p())
                if (wait_until(lock, time_point) == cv_status::timeout)
                    return p();
            return true;
        }

        template<typename Rep, typename Period>
        cv_status wait_for(unique_lock<mutex>& lock, const chrono::duration<Rep, Period>& duration)
        {
            int adjustForInaccuracies = ratio_less_equal<clock_type::period, Period>::value ? 0 : 1;
            return wait_for_impl(lock, (chrono::duration_cast<chrono::milliseconds>(duration).count() + adjustForInaccuracies) / portTICK_RATE_MS);
        }

        template<typename Rep, typename Period, typename Predicate>
        bool wait_for(unique_lock<mutex>& lock, const chrono::duration<Rep, Period>& duration, Predicate p)
        {
            while (!p())
                if (wait_for(lock, duration) == cv_status::timeout)
                    return p();
            return true;
        }

        native_handle_type native_handle()
        {
            return mWaitingFront;
        }

    private:
        cv_status wait_for_impl(unique_lock<mutex>& lock, portTickType duration)
        {
            WaitingEntry entry;
            vSemaphoreCreateBinary(entry.semaphore);
            xSemaphoreTake(entry.semaphore, 0);

            bool timeout;

            {
                lock_guard<mutex> lock(mMutex);

                pushFront(entry);
            }

            {
                Unlock<unique_lock<mutex>> unlock(lock);
                timeout = xSemaphoreTake(entry.semaphore, duration) != pdTRUE;
            }

            if (timeout)
            {
                lock_guard<mutex> lock(mMutex);

                timeout = xSemaphoreTake(entry.semaphore, 0) != pdTRUE;

                if (timeout)
                {
                    erase(entry);
                }
            }

            vSemaphoreDelete(entry.semaphore);

            return timeout ? cv_status::timeout : cv_status::no_timeout;
        }

        void popBack()
        {
            mWaitingBack = mWaitingBack->next;
            if (mWaitingBack)
                mWaitingBack->previous = 0;
            else
                mWaitingFront = 0;
        }

        void pushFront(WaitingEntry& entry)
        {
            if (mWaitingFront)
                mWaitingFront->next = &entry;
            else
                mWaitingBack = &entry;
            entry.previous = mWaitingFront;
            entry.next = 0;
            mWaitingFront = &entry;
        }

        void erase(WaitingEntry& entry)
        {
            if (entry.next != 0)
                entry.next->previous = entry.previous;
            if (entry.previous != 0)
                entry.previous->next = entry.next;
            if (mWaitingFront == &entry)
                mWaitingFront = entry.previous;
            if (mWaitingBack == &entry)
                mWaitingBack = entry.next;
        }

    private:
        native_handle_type mWaitingFront;
        native_handle_type mWaitingBack;
        mutex mMutex;

        condition_variable(const condition_variable&);
        condition_variable& operator=(const condition_variable&);
    };

    class condition_variable_any
    {
        typedef chrono::system_clock clock_type;
        condition_variable mConditionVariable;
        mutex mMutex;

    public:
        condition_variable_any() noexcept = default;
        ~condition_variable_any() noexcept = default;

        void notify_one() noexcept
        {
            lock_guard<mutex> lock(mMutex);
            mConditionVariable.notify_one();
        }

        void notify_all() noexcept
        {
            lock_guard<mutex> lock(mMutex);
            mConditionVariable.notify_all();
        }

        template<typename Lock>
        void wait(Lock& lock)
        {
            unique_lock<mutex> ownLock(mMutex);
            Unlock<Lock> unlock(lock);
            // mMutex must be unlocked before re-locking lock so move
            // ownership of mMutex lock to an object with shorter lifetime.
            unique_lock<mutex> lock2(std::move(ownLock));
            mConditionVariable.wait(lock2);
        }

        template<typename Lock, typename Predicate>
        void wait(Lock& lock, Predicate p)
        {
            while (!p())
                wait(lock);
        }

        template<typename Lock, typename Clock, typename Duration>
        cv_status wait_until(Lock& lock, const chrono::time_point<Clock, Duration>& time_point)
        {
            unique_lock<mutex> ownLock(mMutex);
            Unlock<Lock> unlock(lock);
            // mMutex must be unlocked before re-locking lock so move
            // ownership of mMutex lock to an object with shorter lifetime.
            unique_lock<mutex> lock2(std::move(ownLock));
            return mConditionVariable.wait_until(lock2, time_point);
        }

        template<typename Lock, typename Clock, typename Duration, typename Predicate>
        bool wait_until(Lock& lock, const chrono::time_point<Clock, Duration>& time_point, Predicate p)
        {
            while (!p())
                if (wait_until(lock, time_point) == cv_status::timeout)
                    return p();
            return true;
        }

        template<typename Lock, typename Rep, typename Period>
        cv_status wait_for(Lock& lock, const chrono::duration<Rep, Period>& duration)
        {
            return wait_until(lock, clock_type::now() + duration);
        }

        template<typename Lock, typename Rep, typename Period, typename Predicate>
        bool wait_for(Lock& lock, const chrono::duration<Rep, Period>& duration, Predicate p)
        {
            return wait_until(lock, clock_type::now() + duration, move(p));
        }

    private:
        condition_variable_any(const condition_variable_any&);
        condition_variable_any& operator=(const condition_variable_any&);
    };
}

#endif
