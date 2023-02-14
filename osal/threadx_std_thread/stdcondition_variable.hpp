#ifndef OSAL_THREADX_STD_THREAD_CONDITION_VARIABLE_HPP
#define OSAL_THREADX_STD_THREAD_CONDITION_VARIABLE_HPP

#include "stdmutex.hpp"
#include <chrono>
#include <chrono>

namespace std
{
    enum class cv_status
    {
        no_timeout,
        timeout
    };

    template<typename Lock>
    class Unlock
    {
    public:
        explicit Unlock(Lock& lock)
            : mLock(lock)
        {
            lock.unlock();
        }

        Unlock(const Unlock& other) = delete;
        Unlock& operator=(const Unlock& other) = delete;

        ~Unlock() noexcept
        {
            mLock.lock();
        }

    private:
        Lock& mLock;
    };

    class condition_variable
    {
    private:
        typedef chrono::system_clock clock_type;

        struct WaitingEntry
        {
            TX_SEMAPHORE semaphore;
            WaitingEntry* next;
            WaitingEntry* previous;
        };

    public:
        typedef WaitingEntry* native_handle_type;

        condition_variable() noexcept = default;
        condition_variable(const condition_variable& other) = delete;
        condition_variable& operator=(const condition_variable& other) = delete;
        ~condition_variable() noexcept = default;

        void notify_one() noexcept
        {
            lock_guard<mutex> lock(mMutex);

            if (waitingBack != nullptr)
            {
                UINT result = tx_semaphore_put(&waitingBack->semaphore);
                assert(result == TX_SUCCESS);
                (void)result;
                PopBack();
            }
        }

        void notify_all() noexcept
        {
            lock_guard<mutex> lock(mMutex);

            while (waitingBack != nullptr)
            {
                UINT result = tx_semaphore_put(&waitingBack->semaphore);
                assert(result == TX_SUCCESS);
                (void)result;
                PopBack();
            }
        }

        void wait(unique_lock<mutex>& lock)
        {
            WaitForImpl(lock, TX_WAIT_FOREVER);
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
            return WaitForImpl(lock, (chrono::duration_cast<chrono::milliseconds>(duration).count() + adjustForInaccuracies) * TX_TIMER_TICKS_PER_SECOND / 1000);
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
            return waitingFront;
        }

    private:
        cv_status WaitForImpl(unique_lock<mutex>& lock, ULONG duration)
        {
            WaitingEntry entry;
            tx_semaphore_create(&entry.semaphore, const_cast<char*>(""), 0);

            bool timeout;

            {
                lock_guard<mutex> lock(mMutex);
                PushFront(entry);
            }

            {
                Unlock<unique_lock<mutex>> unlock(lock);
                timeout = tx_semaphore_get(&entry.semaphore, duration) != TX_SUCCESS;
            }

            if (timeout)
            {
                lock_guard<mutex> lock(mMutex);

                timeout = tx_semaphore_get(&entry.semaphore, TX_NO_WAIT) != TX_SUCCESS;

                if (timeout)
                {
                    Erase(entry);
                }
            }

            tx_semaphore_delete(&entry.semaphore);

            return timeout ? cv_status::timeout : cv_status::no_timeout;
        }

        void PopBack()
        {
            waitingBack = waitingBack->next;
            if (waitingBack)
                waitingBack->previous = nullptr;
            else
                waitingFront = nullptr;
        }

        void PushFront(WaitingEntry& entry)
        {
            if (waitingFront)
                waitingFront->next = &entry;
            else
                waitingBack = &entry;
            entry.previous = waitingFront;
            entry.next = nullptr;
            waitingFront = &entry;
        }

        void Erase(WaitingEntry& entry)
        {
            if (entry.next != nullptr)
                entry.next->previous = entry.previous;
            if (entry.previous != nullptr)
                entry.previous->next = entry.next;
            if (waitingFront == &entry)
                waitingFront = entry.previous;
            if (waitingBack == &entry)
                waitingBack = entry.next;
        }

    private:
        native_handle_type waitingFront = nullptr;
        native_handle_type waitingBack = nullptr;
        mutex mMutex;
    };

    class condition_variable_any
    {
        typedef chrono::system_clock clock_type;
        condition_variable mConditionVariable;
        mutex mMutex;

    public:
        condition_variable_any() noexcept = default;
        condition_variable_any(const condition_variable_any& other) = delete;
        condition_variable_any& operator=(const condition_variable_any& other) = delete;
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
    };
}

#endif
