#ifndef OSAL_THREADX_STD_THREAD_MUTEX_HPP
#define OSAL_THREADX_STD_THREAD_MUTEX_HPP

#include "tx_api.h"
#include <cassert>
#include <chrono>
#include <exception>
#include <functional>
#include <system_error>
#include <tuple>
#include <type_traits>

namespace std
{
    class timed_mutex
    {
    public:
        typedef TX_MUTEX native_handle_type;

        timed_mutex() noexcept
        {
            tx_mutex_create(&handle, const_cast<char*>(""), 1);
        }

        timed_mutex(const timed_mutex& other) = delete;
        timed_mutex& operator=(const timed_mutex& other) = delete;

        ~timed_mutex()
        {
            tx_mutex_delete(&handle);
        }

        void lock()
        {
            auto result [[maybe_unused]] = tx_mutex_get(&handle, TX_WAIT_FOREVER);
            assert(result == TX_SUCCESS);
        }

        bool try_lock() noexcept
        {
            return tx_mutex_get(&handle, TX_NO_WAIT) == TX_SUCCESS;
        }

        void unlock()
        {
            auto result [[maybe_unused]] = tx_mutex_put(&handle);
            assert(result == TX_SUCCESS);
        }

        native_handle_type native_handle()
        {
            return handle;
        }

        template<class Rep, class Period>
        bool try_lock_for(const chrono::duration<Rep, Period>& duration)
        {
            auto adjustForInaccuracies = ratio_less_equal<clock_type::period, Period>::value ? 0 : 1;
            return tx_mutex_get(handle, (chrono::duration_cast<chrono::milliseconds>(duration).count() + adjustForInaccuracies) * TX_TIMER_TICKS_PER_SECOND / 1000) == TX_SUCCESS;
        }

        template<class Clock, class Duration>
        bool try_lock_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            return try_lock_for(timePoint - Clock::now());
        }

    private:
        typedef chrono::high_resolution_clock clock_type;

    private:
        native_handle_type handle;
    };

    using recursive_timed_mutex = timed_mutex;

    class mutex
        : private timed_mutex
    {
    public:
        mutex() = default;
        mutex(const mutex& other) = delete;
        mutex& operator=(const mutex& other) = delete;
        ~mutex() = default;

        using timed_mutex::lock;
        using timed_mutex::native_handle_type;
        using timed_mutex::native_handle_type;
        using timed_mutex::try_lock;
        using timed_mutex::unlock;
    };

    class recursive_mutex
        : private recursive_timed_mutex
    {
    public:
        recursive_mutex() = default;
        recursive_mutex(const recursive_mutex&) = delete;
        recursive_mutex& operator=(const recursive_mutex&) = delete;
        ~recursive_mutex() = default;

        using recursive_timed_mutex::lock;
        using recursive_timed_mutex::native_handle_type;
        using recursive_timed_mutex::native_handle_type;
        using recursive_timed_mutex::try_lock;
        using recursive_timed_mutex::unlock;
    };

    struct defer_lock_t { };
    struct try_to_lock_t { };
    struct adopt_lock_t { };

    constexpr defer_lock_t defer_lock { };
    constexpr try_to_lock_t try_to_lock { };
    constexpr adopt_lock_t adopt_lock { };

    template<typename Mutex>
    class lock_guard
    {
    public:
        typedef Mutex mutex_type;

        explicit lock_guard(mutex_type& mutex)
            : mutex(mutex)
        {
            mutex.lock();
        }

        lock_guard(mutex_type& mutex, adopt_lock_t)
            : mutex(mutex)
        {}

        ~lock_guard()
        {
            mutex.unlock();
        }

        lock_guard(const lock_guard&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;

    private:
        mutex_type& mutex;
    };

    template<typename Mutex>
    class unique_lock
    {
    public:
        typedef Mutex mutex_type;

        unique_lock() noexcept = default;

        explicit unique_lock(mutex_type& mutex)
            : mMutex(&mutex)
        {
            lock();
        }

        unique_lock(mutex_type& mutex, defer_lock_t) noexcept
            : mMutex(&mutex)
        {}

        unique_lock(mutex_type& mutex, try_to_lock_t)
            : mMutex(&mutex)
        {
            try_lock();
        }

        unique_lock(mutex_type& mutex, adopt_lock_t)
            : mMutex(&mutex)
            , owns(true)
        {}

        template<typename Clock, typename Duration>
        unique_lock(mutex_type& mutex, const chrono::time_point<Clock, Duration>& timePoint)
            : mMutex(&mutex)
        {
            try_lock_until(timePoint);
        }

        template<typename Rep, typename Period>
        unique_lock(mutex_type& mutex, const chrono::duration<Rep, Period>& duration)
            : mMutex(&mutex)
        {
            try_lock_for(duration);
        }

        unique_lock(const unique_lock& other) = delete;

        unique_lock(unique_lock&& other) noexcept
            : mMutex(other.mMutex)
            , owns(other.owns)
        {
            other.mMutex = nullptr;
            other.owns = false;
        }

        ~unique_lock()
        {
            if (owns)
                unlock();
        }

        unique_lock& operator=(const unique_lock& other) = delete;

        unique_lock& operator=(unique_lock&& other) noexcept
        {
            if (owns)
                unlock();

            mMutex = other.mMutex;
            owns = other.owns;

            other.mMutex = nullptr;
            other.owns = false;

            return *this;
        }

        void lock()
        {
            assert(mMutex != nullptr);
            assert(!owns);

            mMutex->lock();
            owns = true;
        }

        bool try_lock()
        {
            assert(mMutex != nullptr);
            assert(!owns);

            owns = mMutex->try_lock();
            return owns;
        }

        template<typename Clock, typename Duration>
        bool try_lock_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            assert(mMutex != nullptr);
            assert(!owns);

            owns = mMutex->try_lock_until(timePoint);
            return owns;
        }

        template<typename Rep, typename Period>
        bool try_lock_for(const chrono::duration<Rep, Period>& duration)
        {
            assert(mMutex != nullptr);
            assert(!owns);

            owns = mMutex->try_lock_for(duration);
            return owns;
        }

        void unlock()
        {
            assert(mMutex != nullptr);
            assert(owns);

            mMutex->unlock();
            owns = false;
        }

        void swap(unique_lock& other) noexcept
        {
            std::swap(mMutex, other.mMutex);
            std::swap(owns, other.owns);
        }

        mutex_type* release() noexcept
        {
            mutex_type* result = mMutex;
            mMutex = nullptr;
            owns = false;
            return result;
        }

        bool owns_lock() const noexcept
        {
            return owns;
        }

        explicit operator bool() const noexcept
        {
            return owns_lock();
        }

        mutex_type* mutex() const noexcept
        {
            return mMutex;
        }

    private:
        mutex_type* mMutex = nullptr;
        bool owns = false;
    };

    template<typename Mutex>
    inline void swap(unique_lock<Mutex>& x, unique_lock<Mutex>& y) noexcept
    {
        x.swap(y);
    }

    inline int try_lock()
    {
        return -1;
    }

    template<class Lockable1, class... LockableN>
    int try_lock(Lockable1& lock1, LockableN&... lockn)
    {
        unique_lock<Lockable1> l1(lock1, try_to_lock);
        if (!l1)
            return 0;

        int failed_lock = try_lock(lockn...);
        if (failed_lock != -1)
            return failed_lock + 1;
        l1.release();
        return -1;
    }

    template<class Lockable1, class... LockableN>
    int lock_helper(int lock_first, Lockable1& m1, LockableN&... mN)
    {
        if (lock_first == 0)
        {
            unique_lock<Lockable1> l1(m1);
            int failed_lock = try_lock(mN...);
            if (failed_lock == -1)
                return -1;
            else
                return failed_lock + 1;
        }
        else
        {
            int failed_lock = lock_helper(lock_first - 1, mN..., m1);
            if (failed_lock == -1)
                return -1;
            else
                return failed_lock + 1;
        }
    }

    template<class... LockableN>
    void lock(LockableN&... mN)
    {
        int lock_first = 0;

        while (true)
        {
            lock_first = lock_helper(lock_first, mN...);
            if (lock_first == -1)
                return;
            lock_first = lock_first % sizeof...(mN);
        }
    }
}

#endif
