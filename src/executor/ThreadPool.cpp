#include "ThreadPool.h"
#include "../../utils/Logger.h"

ThreadPool::ThreadPool(size_t threadCount) : stop(false) {
    for (size_t i = 0; i < threadCount; ++i) {
        workers.emplace_back(&ThreadPool::workerLoop, this);
    }
    Logger::info("ThreadPool initialized with " + std::to_string(threadCount) + " threads");
}

void ThreadPool::submit(Task task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(std::move(task));
    }
    cv.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        Task task(0, 0, [] {});

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return stop || !taskQueue.empty();
            });

            if (stop && taskQueue.empty())
                return;

            task = std::move(taskQueue.front());
            taskQueue.pop();
        }

        task.execute();
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }

    cv.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable())
            worker.join();
    }

    Logger::info("ThreadPool shutdown completed");
}
