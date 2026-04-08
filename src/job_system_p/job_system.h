#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>

namespace rfct {
    struct jobTracker {
        uint32_t count = 0;
        uint32_t expected = 0;
        std::mutex mtx;
        std::condition_variable cv;

        void increment() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                count++;
            }
            cv.notify_all();
        }

        void addTask() {
            std::lock_guard<std::mutex> lock(mtx);
            expected++;
        }

        void waitUntil(uint32_t target) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return count >= target; });
        }

        void waitAll() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return count >= expected; });
        }
    };
	class jobSystem {
    public:
        static jobSystem& get();
    public:
        jobSystem();
        ~jobSystem();
        void KickJob(std::function<void()> job, jobTracker& counter);
        void Start();
        void Stop();
    private:
        void WorkerThread();
    private:
        std::vector<std::thread> threads;
        std::queue<std::pair<std::function<void()>, jobTracker*>> jobQueue;
        std::mutex queueMutex;
        std::condition_variable cv;
        bool stopThreads = false;
	};
}