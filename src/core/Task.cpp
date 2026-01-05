#include "Task.h"

Task::Task(int id, int priority, TaskFn fn)
    : id(id), priority(priority), fn(fn) {}

void Task::execute() const {
    fn();
}

int Task::getId() const {
    return id;
}

int Task::getPriority() const {
    return priority;
}
