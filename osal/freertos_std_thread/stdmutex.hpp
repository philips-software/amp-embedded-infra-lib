#ifndef OSAL_FREERTOS_STD_THREAD_MUTEX_HPP
#define OSAL_FREERTOS_STD_THREAD_MUTEX_HPP

#include "FreeRTOS.h"
#include "semphr.h"
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
        typedef xSemaphoreHandle native_handle_type;

        constexpr timed_mutex() = default;
        timed_mutex(const timed_mutex& other) = delete;
        timed_mutex& operator=(const timed_mutex& other) = delete;

        ~timed_mutex()
        {
            vSemaphoreDelete(handle);
        }

        void lock()
        {
            auto result [[maybe_unused]] = xSemaphoreTake(handle, portMAX_DELAY);
            assert(result == pdTRUE);
        }

        bool try_lock() noexcept
        {
            return xSemaphoreTake(handle, 0) == pdTRUE;
        }

        void unlock()
        {
            auto result [[maybe_unused]] = xSemaphoreGive(handle);
            assert(result == pdTRUE);
        }

        native_handle_type native_handle()
        {
            return handle;
        }

        template<class Rep, class Period>
        bool try_lock_for(const chrono::duration<Rep, Period>& duration)
        {
            auto adjustForInaccuracies = ratio_less_equal<clock_type::period, Period>::value ? 0 : 1;
            return xSemaphoreTake(handle, (chrono::duration_cast<chrono::milliseconds>(duration).count() + adjustForInaccuracies) / portTICK_RATE_MS) == pdTRUE;
        }

        template <class Clock, class Duration>
        bool try_lock_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            return try_lock_for(timePoint - Clock::now());
        }

    private:
        typedef chrono::high_resolution_clock clock_type;

    private:
        native_handle_type handle = xSemaphoreCreateMutex();
    };

    class recursive_timed_mutex
    {
    public:
        typedef xSemaphoreHandle native_handle_type;

        constexpr recursive_timed_mutex() = default;
        recursive_timed_mutex(const recursive_timed_mutex& other) = delete;
        recursive_timed_mutex& operator=(const recursive_timed_mutex& other) = delete;

        ~recursive_timed_mutex()
        {
            vSemaphoreDelete(handle);
        }

        void lock()
        {
            auto result [[maybe_unused]] = xSemaphoreTakeRecursive(handle, portMAX_DELAY);
            assert(result == pdTRUE);
        }

        bool try_lock() noexcept
        {
            return xSemaphoreTakeRecursive(handle, 0) == pdTRUE;
        }

        void unlock()
        {
            auto result [[maybe_unused]] = xSemaphoreGiveRecursive(handle);
            assert(result == pdTRUE);
        }

        native_handle_type native_handle()
        {
            return handle;
        }

        template<class Rep, class Period>
        bool try_lock_for(const chrono::duration<Rep, Period>& duration)
        {
            auto adjustForInaccuracies = ratio_less_equal<clock_type::period, Period>::value ? 0 : 1;
            return xSemaphoreTakeRecursive(handle, chrono::duration_cast<clock_type::duration>(duration) + adjustForInaccuracies) == pdTRUE;
        }

        template <class Clock, class Duration>
        bool try_lock_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            return try_lock_for(timePoint - Clock::now());
        }

    private:
        typedef chrono::high_resolution_clock clock_type;

    private:
        native_handle_type handle = xSemaphoreCreateRecursiveMutex();
    };

    class mutex
        : private timed_mutex
    {
    public:
        constexpr mutex() = default;
        mutex(const mutex& other) = delete;
        mutex& operator=(const mutex& other) = delete;
        ~mutex() = default;

        using timed_mutex::native_handle_type;
        using timed_mutex::lock;
        using timed_mutex::try_lock;
        using timed_mutex::unlock;
    };

    class recursive_mutex
        : private recursive_timed_mutex
    {
    public:
        constexpr recursive_mutex() = default;
        recursive_mutex(const recursive_mutex&) = delete;
        recursive_mutex& operator=(const recursive_mutex&) = delete;
        ~recursive_mutex() = default;

        using recursive_timed_mutex::native_handle_type;
        using recursive_timed_mutex::lock;
        using recursive_timed_mutex::try_lock;
        using recursive_timed_mutex::unlock;
    };

    // clang-format off
    struct defer_lock_t {};
    struct try_to_lock_t {};
    struct adopt_lock_t {};

    constexpr defer_lock_t defer_lock {};
    constexpr try_to_lock_t try_to_lock {};
    constexpr adopt_lock_t adopt_lock {};

    // clang-format on

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
            , mOwns(true)
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
            , mOwns(other.mOwns)
        {
            other.mMutex = nullptr;
            other.mOwns = false;
        }

        ~unique_lock()
        {
            if (mOwns)
                unlock();
        }

        unique_lock& operator=(const unique_lock& other) = delete;

        unique_lock& operator=(unique_lock&& other) noexcept
        {
            if (mOwns)
                unlock();

            mMutex = other.mMutex;
            mOwns = other.mOwns;

            other.mMutex = nullptr;
            other.mOwns = false;

            return *this;
        }

        void lock()
        {
            assert(mMutex != nullptr);
            assert(!mOwns);

            mMutex->lock();
            mOwns = true;
        }

        bool try_lock()
        {
            assert(mMutex != nullptr);
            assert(!mOwns);

            mOwns = mMutex->try_lock();
            return mOwns;
        }

        template<typename Clock, typename Duration>
        bool try_lock_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            assert(mMutex != nullptr);
            assert(!mOwns);

            mOwns = mMutex->try_lock_until(timePoint);
            return mOwns;
        }

        template<typename Rep, typename Period>
        bool try_lock_for(const chrono::duration<Rep, Period>& duration)
        {
            assert(mMutex != nullptr);
            assert(!mOwns);

            mOwns = mMutex->try_lock_for(duration);
            return mOwns;
        }

        void unlock()
        {
            assert(mMutex != nullptr);
            assert(mOwns);

            mMutex->unlock();
            mOwns = false;
        }

        void swap(unique_lock& other) noexcept
        {
            std::swap(mMutex, other.mMutex);
            std::swap(mOwns, other.mOwns);
        }

        mutex_type* release() noexcept
        {
            mutex_type* result = mMutex;
            mMutex = 0;
            mOwns = false;
            return result;
        }

        bool owns_lock() const noexcept
        {
            return mOwns;
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
        bool mOwns = false;
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
