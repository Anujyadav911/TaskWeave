#include "Task.h"

#include <thread>

Task::Task(int id, TaskPriority priority, std::function<void()> fn)
    : id(id),
      priority(priority),
      fn(std::move(fn)),
      state(TaskState::CREATED),
      enqueueTime(),
      startTime(),
      endTime(),
      threadId() {}

bool Task::canTransition(TaskState from, TaskState to) const {
    switch (from) {
        case TaskState::CREATED:
            return to == TaskState::READY;

        case TaskState::READY:
            return to == TaskState::RUNNING;

        case TaskState::RUNNING:
            return to == TaskState::COMPLETED ||
                   to == TaskState::FAILED;

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
    }

    endTime = std::chrono::steady_clock::now();
    threadId = std::this_thread::get_id();
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
