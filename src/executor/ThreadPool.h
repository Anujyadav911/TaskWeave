#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <memory>

#include "../scheduler/Scheduler.h"

class ThreadPool {
public:
    ThreadPool(size_t threadCount, std::shared_ptr<Scheduler> scheduler);
    ~ThreadPool();

    void start();

private:
    void workerLoop();

    std::vector<std::thread> workers;
    std::shared_ptr<Scheduler> scheduler;

    std::atomic<bool> stop;
    std::mutex mtx;
    std::condition_variable cv;
};
