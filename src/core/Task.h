#pragma once
#include <functional>
#include <chrono>
#include "TaskState.h"

enum class TaskPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};

class Task {
public:
    Task(int id, TaskPriority priority, std::function<void()> fn);

    void markReady();
    void execute();

    int getId() const;
    TaskPriority getPriority() const;
    TaskState getState() const;
    std::chrono::steady_clock::time_point getEnqueueTime() const;

private:
    int id;
    TaskPriority priority;
    std::function<void()> fn;

    TaskState state;
    std::chrono::steady_clock::time_point enqueueTime;
};
