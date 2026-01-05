#include <iostream>
#include <thread>

#include "executor/ThreadPool.h"
#include "scheduler/PriorityScheduler.h"
#include "../utils/Logger.h"

int main() {
    Logger::info("TaskWeave Engine Starting (Phase 2)");

    auto scheduler = std::make_shared<PriorityScheduler>();
    ThreadPool pool(3, scheduler);

    scheduler->submit(Task(1, TaskPriority::LOW, [] {
        std::cout << "LOW priority task\n";
    }));

    scheduler->submit(Task(2, TaskPriority::HIGH, [] {
        std::cout << "HIGH priority task\n";
    }));

    scheduler->submit(Task(3, TaskPriority::MEDIUM, [] {
        std::cout << "MEDIUM priority task\n";
    }));

    pool.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
