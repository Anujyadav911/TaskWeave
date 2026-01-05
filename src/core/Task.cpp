#include "Task.h"

Task::Task(int id, int priority, std::function<void()> work)
    : id(id), priority(priority), state(TaskState::PENDING), work(work) {}

int Task::getId() const {
    return id;
}

int Task::getPriority() const {
    return priority;
}

TaskState Task::getState() const {
    return state;
}

void Task::execute() {
    state = TaskState::RUNNING;
    try {
        work();
        state = TaskState::COMPLETED;
    } catch (...) {
        state = TaskState::FAILED;
    }
}
