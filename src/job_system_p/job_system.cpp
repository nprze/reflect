#include "job_system.h"

rfct::jobSystem::jobSystem(size_t numThreads)
{
    threads.resize(numThreads);
}

rfct::jobSystem::~jobSystem()
{
    Stop();
}

void rfct::jobSystem::KickJob(std::function<void()> job, jobTracker& counter)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        jobQueue.push({ job, &counter });
    }
    cv.notify_one();
}

void rfct::jobSystem::Start()
{
    for (auto& thread : threads) {
        thread = std::thread(&jobSystem::WorkerThread, this);
    }
}

void rfct::jobSystem::Stop()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopThreads = true;
    }
    cv.notify_all();

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void rfct::jobSystem::WorkerThread()
{
    while (true) {
        std::function<void()> job;
        jobTracker* counter = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !jobQueue.empty() || stopThreads; });

            if (stopThreads && jobQueue.empty()) {
                return;
            }

            job = jobQueue.front().first;
            counter = jobQueue.front().second;
            jobQueue.pop();
        }

        job(); 

        if (counter) {
            counter->increment();
        }
    }
}
