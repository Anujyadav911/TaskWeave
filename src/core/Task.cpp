#include "Task.h"

Task::Task(int id, TaskPriority priority, std::function<void()> fn)
    : id(id), priority(priority), fn(std::move(fn)), state(TaskState::CREATED) {}

void Task::markReady() {
    state = TaskState::READY;
    enqueueTime = std::chrono::steady_clock::now();
}

void Task::execute() {
    state = TaskState::RUNNING;
    fn();
    state = TaskState::COMPLETED;
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
