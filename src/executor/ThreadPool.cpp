#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadCount,
                       std::shared_ptr<Scheduler> scheduler)
    : scheduler(std::move(scheduler)), stop(false) {
    workers.reserve(threadCount);
}

void ThreadPool::start() {
    for (size_t i = 0; i < workers.capacity(); ++i) {
        workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

void ThreadPool::workerLoop() {
    while (!stop) {
        if (!scheduler->empty()) {
            Task task = scheduler->getNextTask();
            task.execute();
        } else {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::milliseconds(50));
        }
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    cv.notify_all();
    for (auto& t : workers) {
        if (t.joinable())
            t.join();
    }
}
