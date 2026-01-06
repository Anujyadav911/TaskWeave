#include "Task.h"

#include <thread>

Task::Task(int id,
           TaskPriority priority,
           std::function<void()> fn,
           int maxRetries)
    : id(id),
      priority(priority),
      fn(std::move(fn)),
      state(TaskState::CREATED),
      enqueueTime(),
      startTime(),
      endTime(),
      threadId(),
      retryCount(0),
      maxRetries(maxRetries) {}

bool Task::canTransition(TaskState from, TaskState to) const {
    switch (from) {
        case TaskState::CREATED:
            return to == TaskState::READY;

        case TaskState::READY:
            return to == TaskState::RUNNING ||
                   to == TaskState::READY;

        case TaskState::RUNNING:
            return to == TaskState::COMPLETED ||
                   to == TaskState::FAILED;

        case TaskState::FAILED:
            return to == TaskState::RETRYING;

        case TaskState::RETRYING:
            return to == TaskState::READY;

        default:
            return false;
    }
}

void Task::markReady() {
    if (!canTransition(state, TaskState::READY))
        return;

    state = TaskState::READY;
    enqueueTime = std::chrono::steady_clock::now();
}

void Task::execute() {
    if (!canTransition(state, TaskState::RUNNING))
        return;

    state = TaskState::RUNNING;
    startTime = std::chrono::steady_clock::now();

    try {
        fn();
        state = TaskState::COMPLETED;
    } catch (...) {
        state = TaskState::FAILED;
        endTime = std::chrono::steady_clock::now();
        threadId = std::this_thread::get_id();
        throw;
    }

    endTime = std::chrono::steady_clock::now();
    threadId = std::this_thread::get_id();
}

bool Task::shouldRetry() const {
    return retryCount < maxRetries;
}

void Task::markRetry() {
    if (!shouldRetry())
        return;

    if (!canTransition(state, TaskState::RETRYING))
        return;

    state = TaskState::RETRYING;
    ++retryCount;

    // Move back to READY and capture a new enqueue time
    markReady();
}

void Task::markFailed() {
    state = TaskState::FAILED;
}

int Task::getId() const {
    return id;
}

TaskPriority Task::getPriority() const {
    return priority;
}

TaskState Task::getState() const {
    return state;
}

std::chrono::steady_clock::time_point Task::getEnqueueTime() const {
    return enqueueTime;
}

std::chrono::steady_clock::time_point Task::getStartTime() const {
    return startTime;
}

std::chrono::steady_clock::time_point Task::getEndTime() const {
    return endTime;
}

std::thread::id Task::getThreadId() const {
    return threadId;
}

int Task::getRetryCount() const {
    return retryCount;
}

int Task::getMaxRetries() const {
    return maxRetries;
}
