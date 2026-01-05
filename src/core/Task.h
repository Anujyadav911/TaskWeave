#ifndef TASK_H
#define TASK_H

#include <functional>
#include <string>
#include "TaskState.h"

class Task {
private:
    int id;
    int priority;
    TaskState state;
    std::function<void()> work;

public:
    Task(int id, int priority, std::function<void()> work);

    int getId() const;
    int getPriority() const;
    TaskState getState() const;

    void execute();
};

#endif
