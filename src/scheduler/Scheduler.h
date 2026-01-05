#pragma once
#include "../core/Task.h"

class Scheduler {
public:
    virtual void submit(Task task) = 0;
    virtual Task getNextTask() = 0;
    virtual bool empty() const = 0;
    virtual ~Scheduler() = default;
};
