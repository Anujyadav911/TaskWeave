#pragma once

#include "Scheduler.h"
#include <queue>
#include <mutex>

class RoundRobinScheduler : public Scheduler {
private:
    std::queue<Task> taskQueue;
    mutable std::mutex queueMutex;

public:
    void submit(Task task) override;
    Task getNextTask() override;
    bool empty() const override;
};
