#include "ThreadPool.h"
#include "../scheduler/RoundRobinScheduler.h"
#include "../../utils/Metrics.h"

ThreadPool::ThreadPool(size_t threadCount)
    : scheduler(std::make_shared<RoundRobinScheduler>()), stop(false) {
    workers.reserve(threadCount);
    start();
}

ThreadPool::ThreadPool(size_t threadCount,
                       std::shared_ptr<Scheduler> scheduler)
    : scheduler(std::move(scheduler)), stop(false) {
    workers.reserve(threadCount);
    start();
}

void ThreadPool::start() {
    if (workers.size() < workers.capacity()) {
        for (size_t i = workers.size(); i < workers.capacity(); ++i) {
            workers.emplace_back(&ThreadPool::workerLoop, this);
        }
    }
}

void ThreadPool::submit(Task task) {
    if (workers.empty()) {
        start();
    }
    task.markReady();
    scheduler->submit(std::move(task));
}

void ThreadPool::workerLoop() {
    while (!stop) {
        if (!scheduler->empty()) {
            Task task = scheduler->getNextTask();
            task.execute();
            Metrics::instance().recordTask(task);
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
