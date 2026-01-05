#pragma once
#include <functional>

class Task {
public:
    using TaskFn = std::function<void()>;

    Task(int id, int priority, TaskFn fn);

    void execute() const;

    int getId() const;
    int getPriority() const;

private:
    int id;
    int priority;
    TaskFn fn;
};
