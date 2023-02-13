#ifndef OSAL_FREERTOS_STD_THREAD_THREAD_HPP
#define OSAL_FREERTOS_STD_THREAD_THREAD_HPP

#include "FreeRTOS.h"
#include "infra/util/Function.hpp"
#include "task.h"
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
        typedef xTaskHandle native_handle_type;

        class id
        {
        public:
            id() noexcept
                : mThreadHandle()
            {}

            explicit id(native_handle_type handle)
                : mThreadHandle(handle)
            {}

            bool operator==(thread::id other) const noexcept
            {
                return mThreadHandle == other.mThreadHandle;
            }

            bool operator!=(id other) const noexcept
            {
                return mThreadHandle != other.mThreadHandle;
            }

            bool operator<(id other) const noexcept
            {
                return mThreadHandle < other.mThreadHandle;
            }

            bool operator>(id other) const noexcept
            {
                return mThreadHandle > other.mThreadHandle;
            }

            bool operator<=(id other) const noexcept
            {
                return mThreadHandle <= other.mThreadHandle;
            }

            bool operator>=(id other) const noexcept
            {
                return mThreadHandle >= other.mThreadHandle;
            }

            template<class T, class Traits>
            friend basic_ostream<T, Traits>& operator<<(basic_ostream<T, Traits>& out, thread::id id)
            {
                if (id == thread::id())
                    return out << "thread::id of a non-executing thread";
                else
                    return out << id.mThreadHandle;
            }

        private:
            friend class thread;
            friend class std::hash<id>;
            native_handle_type mThreadHandle;
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
            ThreadData(F&& f, Args&&... args)
                : referenceCount(2)
                , workerFunction(std::forward<F>(f), std::forward<Args...>(args)...)
                , deleteByThreadFunction(true)
            {}

            int referenceCount;
            infra::Function<void()> workerFunction;
            mutex joinMutex;
            bool deleteByThreadFunction;
            condition_variable condition;
        };

        static void threadFunction(void* d)
        {
            ThreadData* data = reinterpret_cast<ThreadData*>(d);
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

            if (deleteByThreadFunction)
                vTaskDelete(nullptr);
            else
                vTaskSuspend(nullptr);
        }

    public:
        template<typename F, typename... Args>
        explicit thread(F&& f, Args&&... args)
        {
            unsigned portSHORT stackSize = 2048;
            unsigned portBASE_TYPE priority = 3;

            mThreadData = new ThreadData(std::forward<F>(f), std::forward<Args>(args)...);

            portBASE_TYPE result = xTaskCreate(&threadFunction, "", stackSize, mThreadData, priority, &mThreadId.mThreadHandle);
            if (result != pdPASS)
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
            std::swap(mThreadId, other.mThreadId);
        }

        bool joinable() const noexcept
        {
            return mThreadId != id();
        }

        void join()
        {
            if (!joinable())
                std::abort();

            {
                unique_lock<mutex> lock(mThreadData->joinMutex);

                if (mThreadData->referenceCount == 2)
                    mThreadData->deleteByThreadFunction = false;

                while (mThreadData->referenceCount > 1)
                    mThreadData->condition.wait(lock);

                --mThreadData->referenceCount;

                if (!mThreadData->deleteByThreadFunction)
                    vTaskDelete(mThreadId.mThreadHandle);
            }

            delete mThreadData;
            mThreadData = 0;
            mThreadId = id();
        }

        void detach()
        {
            if (!joinable())
                std::abort();

            unique_lock<mutex> lock(mThreadData->joinMutex);

            if (--mThreadData->referenceCount == 0)
            {
                if (!mThreadData->deleteByThreadFunction)
                    vTaskDelete(mThreadId.mThreadHandle);

                lock.unlock();

                delete mThreadData;
            }

            mThreadData = nullptr;
            mThreadId = id();
        }

        thread::id get_id() const noexcept
        {
            return mThreadId;
        }

        native_handle_type native_handle()
        {
            return mThreadId.mThreadHandle;
        }

        static unsigned int hardware_concurrency() noexcept
        {
            return 0;
        }

    private:
        ThreadData* mThreadData;
        id mThreadId;
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
            return hash<thread::native_handle_type>()(id.mThreadHandle);
        }
    };

    namespace this_thread
    {
        inline thread::id get_id() noexcept
        {
            return thread::id(xTaskGetCurrentTaskHandle());
        }

        inline void yield() noexcept
        {
            taskYIELD();
        }

        template<typename Rep, typename Period>
        inline void sleep_for(const chrono::duration<Rep, Period>& duration)
        {
            int adjustForInaccuracies = ratio_less_equal<std::milli, Period>::value ? 0 : 1;
            portTickType delay = (chrono::duration_cast<chrono::milliseconds>(duration).count() + adjustForInaccuracies) / portTICK_RATE_MS;
            vTaskDelay(delay);
        }

        template<typename Clock, typename Duration>
        inline void sleep_until(const chrono::time_point<Clock, Duration>& timePoint)
        {
            sleep_for(timePoint - Clock::now());
        }
    }
}

#endif
