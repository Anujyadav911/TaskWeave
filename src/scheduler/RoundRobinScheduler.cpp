#include "RoundRobinScheduler.h"

void RoundRobinScheduler::submit(Task task) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(std::move(task));
}

Task RoundRobinScheduler::getNextTask() {
    std::lock_guard<std::mutex> lock(queueMutex);

    Task task = std::move(taskQueue.front());
    taskQueue.pop();
    return task;
}

bool RoundRobinScheduler::empty() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.empty();
}
