#pragma once
#include "Scheduler.h"
#include <queue>
#include <mutex>

struct TaskComparator {
    bool operator()(const Task& a, const Task& b) const {
        if (a.getPriority() != b.getPriority())
            return static_cast<int>(a.getPriority()) <
                   static_cast<int>(b.getPriority());

        return a.getEnqueueTime() > b.getEnqueueTime();
    }
};

class PriorityScheduler : public Scheduler {
public:
    void submit(Task task) override;
    Task getNextTask() override;
    bool empty() const override;

private:
    mutable std::mutex mtx;
    std::priority_queue<Task, std::vector<Task>, TaskComparator> pq;
};
