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
    if (!accepting) {
        return;
    }

    if (workers.empty()) {
        start();
    }
    task.markReady();
    scheduler->submit(std::move(task));
    cv.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        if (!scheduler->empty()) {
            Task task = scheduler->getNextTask();
            try {
                task.execute();
                Metrics::instance().recordTask(task);
            } catch (...) {
                if (task.shouldRetry()) {
                    task.markRetry();
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(50 * task.getRetryCount()));
                    scheduler->submit(task);
                } else {
                    task.markFailed();
                    Metrics::instance().recordTask(task);
                }
            }
        } else {
            std::unique_lock<std::mutex> lock(mtx);
            if (stop) {
                break;
            }
            cv.wait_for(lock, std::chrono::milliseconds(50));
            if (stop && scheduler->empty()) {
                break;
            }
        }
    }
}

void ThreadPool::shutdown() {
    accepting = false;
    stop = true;
    cv.notify_all();
    for (auto& t : workers) {
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::shutdownNow() {
    accepting = false;
    stop = true;
    cv.notify_all();
    for (auto& t : workers) {
        if (t.joinable())
            t.join();
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}
