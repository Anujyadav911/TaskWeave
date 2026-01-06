#include <iostream>
#include <thread>
#include <memory>
#include <chrono>

// Core
#include "core/Task.h"

// Executor
#include "executor/ThreadPool.h"

// Schedulers
#include "scheduler/PriorityScheduler.h"
#include "scheduler/RoundRobinScheduler.h"

// Utils
#include "../utils/Logger.h"
#include "../utils/Metrics.h"

void runPhase1() {
    Logger::info("===== PHASE 1: Basic ThreadPool Execution =====");

    ThreadPool pool(3);

    for (int i = 1; i <= 5; i++) {
        pool.submit(Task(
            i,
            TaskPriority::MEDIUM,
            [i]() {
                std::cout << "[Phase 1] Task " << i
                          << " running on thread "
                          << std::this_thread::get_id()
                          << std::endl;
            }
        ));
    }
}

void runPhase2() {
    Logger::info("===== PHASE 2: Priority Scheduler =====");

    auto scheduler = std::make_shared<PriorityScheduler>();
    ThreadPool pool(3, scheduler);

    pool.submit(Task(
        1, TaskPriority::LOW,
        []() { std::cout << "[Phase 2] LOW priority task\n"; }
    ));

    pool.submit(Task(
        2, TaskPriority::HIGH,
        []() { std::cout << "[Phase 2] HIGH priority task\n"; }
    ));

    pool.submit(Task(
        3, TaskPriority::MEDIUM,
        []() { std::cout << "[Phase 2] MEDIUM priority task\n"; }
    ));
}

void runPhase3() {
    Logger::info("==== PHASE 3: Round Robin Scheduler ====");

    auto scheduler = std::make_shared<RoundRobinScheduler>();
    ThreadPool pool(2, scheduler);

    for (int i = 1; i <= 6; ++i) {
        pool.submit(Task(
            i,
            TaskPriority::MEDIUM,
            [i]() {
                std::cout << "[Phase 3] Task " << i
                          << " executing on thread "
                          << std::this_thread::get_id()
                          << std::endl;
            }
        ));
    }
        // Phase 6 demo: task that fails twice, then succeeds on 3rd attempt
        static int attempts = 0;
        pool.submit(Task(
            42,
            TaskPriority::HIGH,
            [] {
                attempts++;
                std::cout << "[Phase 6] Task 42 attempt " << attempts << std::endl;
                if (attempts < 3) {
                    throw std::runtime_error("simulated failure");
                }
            },
            3 // maxRetries
        ));

    // 🔥 CRITICAL LINE (allow workers to run)
    std::this_thread::sleep_for(std::chrono::seconds(1));
}


int main() {
    Logger::info("TaskWeave Engine Starting");

    runPhase1();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    runPhase2();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    runPhase3();

    Logger::info("TaskWeave Engine Shutdown");

    Metrics::instance().printSummary();
    return 0;
}
