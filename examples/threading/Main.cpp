#include "osal/Osal.hpp"
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

int main()
{
    osal::Init();

    uint32_t thread_counter{};
    std::mutex m;
    std::condition_variable cv;

    std::thread cnt([&] {
        while (true)
        {
            ++thread_counter;
            if (thread_counter > 2)
                cv.notify_one();
            std::this_thread::sleep_for(5s);
        }
    });

    std::thread t([&] {
        while (true)
        {
            std::cout << "Hello! " << thread_counter << std::endl;
            std::this_thread::sleep_for(1s);
        }
    });

    std::thread t2([&] {
        std::once_flag once;
        while (true)
        {
            std::unique_lock lck(m);
            cv.wait(lck);

            std::cout << "T2 entry" << std::endl;

            std::call_once(once,
                [] { std::cout << "Thread 2" << std::endl; });

            lck.unlock();
        }
    });

    osal::Run();
}
