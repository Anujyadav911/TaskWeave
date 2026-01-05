#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "../core/Task.h"

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount);
    ~ThreadPool();

    void submit(Task task);

private:
    void workerLoop();

    std::vector<std::thread> workers;
    std::queue<Task> taskQueue;

    std::mutex queueMutex;
    std::condition_variable cv;

    std::atomic<bool> stop;
};
