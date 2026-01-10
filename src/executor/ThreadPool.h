#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>

#include "../scheduler/Scheduler.h"

class ThreadPool {
public:
    ThreadPool(size_t threadCount);
    ThreadPool(size_t threadCount, std::shared_ptr<Scheduler> scheduler);
    ~ThreadPool();

    void start();
    void submit(Task task);
    void shutdown();      // graceful: finish queued work, stop accepting
    void shutdownNow();   // force: stop immediately
    size_t getSize() const { return workers.size(); }

private:
    void workerLoop();

    std::vector<std::thread> workers;
    std::shared_ptr<Scheduler> scheduler;

    std::atomic<bool> stop;
    std::atomic<bool> accepting{true};
    std::mutex mtx;
    std::condition_variable cv;
};
