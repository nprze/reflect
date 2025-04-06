#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
namespace rfct {
    struct jobTracker {
        uint32_t count = 0;
        std::mutex mtx;
        std::condition_variable cv;

        void increment() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                count++;
                RFCT_ASSERT(count < 4);
            }
            cv.notify_all();
        }

        void waitUntil(uint32_t target) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return count >= target; });
        }
    };
	class jobSystem {
    public:
        jobSystem(size_t numThreads);
        ~jobSystem();

        void KickJob(std::function<void()> job, jobTracker& counter);

        void Start();

        void Stop();

    private:
        void WorkerThread();

        std::vector<std::thread> threads;
        std::queue<std::pair<std::function<void()>, jobTracker*>> jobQueue;
        std::mutex queueMutex;
        std::condition_variable cv;
        bool stopThreads = false;

	};
}