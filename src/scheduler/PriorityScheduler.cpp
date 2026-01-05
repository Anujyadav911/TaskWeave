#include "PriorityScheduler.h"

void PriorityScheduler::submit(Task task) {
    std::lock_guard<std::mutex> lock(mtx);
    task.markReady();
    pq.push(std::move(task));
}

Task PriorityScheduler::getNextTask() {
    std::lock_guard<std::mutex> lock(mtx);
    Task task = std::move(pq.top());
    pq.pop();
    return task;
}

bool PriorityScheduler::empty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return pq.empty();
}
