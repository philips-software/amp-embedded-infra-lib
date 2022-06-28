#ifndef OSAL_THREADX_STD_THREAD_THREAD_HPP
#define OSAL_THREADX_STD_THREAD_THREAD_HPP

#include "infra/util/Function.hpp"
#include "tx_api.h"
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <memory>
#include <mutex>

namespace std
{
    class thread
    {
    public:
        typedef TX_THREAD* native_handle_type;

        class id
        {
        public:
            id() noexcept = default;

            explicit id(native_handle_type handle)
                : threadHandle(handle)
            {}

            bool operator==(thread::id other) const noexcept
            {
                return threadHandle == other.threadHandle;
            }

            bool operator!=(id other) const noexcept
            {
                return threadHandle != other.threadHandle;
            }

            bool operator<(id other) const noexcept
            {
                return threadHandle < other.threadHandle;
            }

            bool operator>(id other) const noexcept
            {
                return threadHandle > other.threadHandle;
            }

            bool operator<=(id other) const noexcept
            {
                return threadHandle <= other.threadHandle;
            }

            bool operator>=(id other) const noexcept
            {
                return threadHandle >= other.threadHandle;
            }

            template<class T, class Traits>
            friend basic_ostream<T, Traits>& operator<<(basic_ostream<T, Traits>& out, thread::id id)
            {
                if (id == thread::id())
                    return out << "thread::id of a non-executing thread";
                else
                    return out << id.threadHandle;
            }

        private:
            friend class thread;
            friend class std::hash<id>;
            native_handle_type threadHandle = nullptr;
        };

    public:
        thread() noexcept = default;
        thread(thread& other) = delete;
        thread(const thread& other) = delete;

        thread(thread&& other) noexcept
        {
            swap(other);
        }

    private:
        struct ThreadData
        {
            template<class F, class... Args>
            ThreadData(void* stack, F&& f, Args&&... args)
                : referenceCount(2)
                , stack(stack)
                , workerFunction(std::forward<F>(f), std::forward<Args...>(args)...)
                , deleteByThreadFunction(true)
                , txThread()
            {}

            ThreadData(const ThreadData& other) = delete;
            ThreadData& operator=(const ThreadData& other) = delete;

            ~ThreadData()
            {
                free(stack);
            }

            int referenceCount;
            void* stack;
            infra::Function<void()> workerFunction;
            mutex joinMutex;
            bool deleteByThreadFunction;
            condition_variable condition;
            TX_THREAD txThread;
        };

        static void ThreadFunction(ULONG arg);

    public:
        template<typename F, typename... Args>
        explicit thread(F&& f, Args&&... args)
        {
            ULONG stackSize = 3072;
            void* stack = malloc(stackSize);
            UINT priority = 1;

            threadData = new (std::nothrow) ThreadData(stack, std::forward<F>(f), std::forward<Args...>(args)...);
            threadId.threadHandle = &threadData->txThread;
            
            UINT result = tx_thread_create(threadId.threadHandle, const_cast<char*>(""), &ThreadFunction, reinterpret_cast<ULONG>(threadData), threadData->stack, stackSize, priority, priority, TX_NO_TIME_SLICE, TX_AUTO_START);
            if (result != TX_SUCCESS)
                std::abort();
        }

        ~thread()
        {
            if (joinable())
                std::abort();
        }

        thread& operator=(const thread& other) = delete;

        thread& operator=(thread&& other) noexcept
        {
            if (joinable())
                std::abort();

            swap(other);
            return *this;
        }

        void swap(thread& other) noexcept
        {
            std::swap(threadId, other.threadId);
            std::swap(threadData, other.threadData);
        }

        bool joinable() const noexcept
        {
            return threadId != id();
        }

        void join()
        {
            if (!joinable())
                std::abort();

            {
                unique_lock<mutex> lock(threadData->joinMutex);

                if (threadData->referenceCount == 2)
                    threadData->deleteByThreadFunction = false;

                while (threadData->referenceCount > 1)
                {
                    threadData->condition.wait(lock);
                }

                --threadData->referenceCount;

                if (!threadData->deleteByThreadFunction)
                    tx_thread_delete(threadId.threadHandle);
            }

            delete threadData;
            threadData = nullptr;
            threadId = id();
        }

        void detach()
        {
            if (!joinable())
                std::abort();

            unique_lock<mutex> lock(threadData->joinMutex);

            if (--threadData->referenceCount == 0)
            {
                if (!threadData->deleteByThreadFunction)
                    tx_thread_delete(threadId.threadHandle);

                lock.unlock();

                delete threadData;
            }

            threadData = nullptr;
            threadId = id();
        }

        thread::id get_id() const noexcept
        {
            return threadId;
        }

        native_handle_type native_handle()
        {
            return threadId.threadHandle;
        }

        static unsigned int hardware_concurrency() noexcept
        {
            return 0;
        }

    private:
        ThreadData* threadData;
        id threadId;
    };

    inline void swap(thread& x, thread& y) noexcept
    {
        x.swap(y);
    }

    template<>
    struct hash<thread::id>
    {
        typedef thread::id argument_type;
        typedef size_t result_type;

        size_t operator()(const thread::id& id) const noexcept
        {
            return hash<thread::native_handle_type>()(id.threadHandle);
        }
    };

    namespace this_thread
    {
        inline thread::id get_id() noexcept
        {
            return thread::id(tx_thread_identify());
        }

        inline void yield() noexcept
        {
            tx_thread_relinquish();
        }

        template<typename Rep, typename Period>
        inline void sleep_for(const chrono::duration<Rep, Period>& duration)
        {
            int adjustForInaccuracies = ratio_less_equal<std::milli, Period>::value ? 0 : 1;
            uint32_t delay = (chrono::duration_cast<chrono::milliseconds>(duration).count() + adjustForInaccuracies) * TX_TIMER_TICKS_PER_SECOND / 1000;
            tx_thread_sleep(delay);
        }

        template<typename Clock, typename Duration>
        inline void sleep_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            sleep_for(timePoint - Clock::now());
        }
    }

    inline void thread::ThreadFunction(ULONG arg)
    {
        ThreadData* data = reinterpret_cast<ThreadData*>(arg);
        data->workerFunction();
        data->workerFunction = nullptr;

        unique_lock<mutex> lock(data->joinMutex);
        bool deleteByThreadFunction;

        if (--data->referenceCount == 0)
        {
            deleteByThreadFunction = data->deleteByThreadFunction;
            lock.unlock();
            delete data;
        }
        else
        {
            data->deleteByThreadFunction = false;
            deleteByThreadFunction = false;
            lock.unlock();
            data->condition.notify_all();
        }

        tx_thread_terminate(this_thread::get_id().threadHandle);

        if (deleteByThreadFunction) 
            tx_thread_delete(this_thread::get_id().threadHandle);
    }
}

#endif
