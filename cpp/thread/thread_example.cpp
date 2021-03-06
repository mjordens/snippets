#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <memory>

#include "thread_example.h"

using namespace std::literals;

namespace {

    struct ThreadKiller {
        std::mutex _mutex;
        std::condition_variable _cv;
        bool _killThread{false};
    };

    struct ThreadKiller _tk;

    constexpr const char* threadName = "Thread_1";
}

ThreadExample::ThreadExample()
{
}

ThreadExample::~ThreadExample()
{
    {
        std::lock_guard<std::mutex> lock(_tk._mutex);
        _tk._killThread=true;
    }
    std::cout << "Destructor> notifying" << std::endl;
    _tk._cv.notify_all();
}

void ThreadExample::create_and_detach_thread()
{
    std::thread ( []()
    {
        // Make sure thread name is not too long
        pthread_setname_np(pthread_self(), threadName);

        // This obtains the mutex lock
        std::unique_lock<std::mutex> lock(_tk._mutex);

        while (true) {
            // Do things periodically every 5 seconds
            std::cout << threadName << "> doing things" << std::endl;

            // Bail out on destruction. Using predicate to prevent spurious wakeups
            if ( _tk._cv.wait_for(lock, std::chrono::seconds(5), [] {return _tk._killThread;} )) {
                std::cout << threadName << "> got killed" << std::endl;
                break;
            }
        }
    }).detach();
}


int main() {
    {
        auto p = std::make_unique<ThreadExample>();
        p->create_and_detach_thread();
        std::this_thread::sleep_for(500ms);
    }
}
