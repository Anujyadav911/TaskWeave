#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include <atomic>
#include <csignal>
#include <string>

// Core
#include "../src/core/Task.h"
#include "../src/core/EngineState.h"
#include "../core/TaskLoader.h"
#include "../core/TaskRegistry.h"

// Executor
#include "../src/executor/ThreadPool.h"

// Schedulers
#include "../src/scheduler/PriorityScheduler.h"
#include "../src/scheduler/RoundRobinScheduler.h"

// API
#include "../api/ApiServer.h"

// Utils
#include "../utils/Logger.h"
#include "../utils/Metrics.h"
#include "../utils/Config.h"
#include "../utils/Database.h"

static std::atomic<EngineState> g_engineState{EngineState::RUNNING};
static std::atomic<bool> g_shutdownRequested{false};

void signalHandler(int signum) {
    (void)signum;
    if (!g_shutdownRequested.exchange(true)) {
        Logger::warn("Shutdown signal received (SIGINT/SIGTERM). Engine state -> SHUTTING_DOWN");
        g_engineState.store(EngineState::SHUTTING_DOWN);
    }
}

// ---------------- PHASE 1 ----------------
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

// ---------------- PHASE 2 ----------------
void runPhase2() {
    Logger::info("===== PHASE 2: Priority Scheduler =====");

    auto scheduler = std::make_shared<PriorityScheduler>();
    ThreadPool pool(3, scheduler);

    pool.submit(Task(1, TaskPriority::LOW, [] {
        std::cout << "[Phase 2] LOW priority task\n";
    }));

    pool.submit(Task(2, TaskPriority::HIGH, [] {
        std::cout << "[Phase 2] HIGH priority task\n";
    }));

    pool.submit(Task(3, TaskPriority::MEDIUM, [] {
        std::cout << "[Phase 2] MEDIUM priority task\n";
    }));
}

// ---------------- PHASE 3 + 6 ----------------
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

    // Phase 6 demo: retry logic
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
        3
    ));

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// ---------------- PHASE 7 ----------------
void runPhase7() {
    Logger::info("===== PHASE 7: Config-Driven Engine =====");

    Config& cfg = Config::instance();

    std::shared_ptr<Scheduler> scheduler;
    if (cfg.getScheduler() == "priority") {
        scheduler = std::make_shared<PriorityScheduler>();
    } else {
        scheduler = std::make_shared<RoundRobinScheduler>();
    }

    ThreadPool pool(static_cast<size_t>(cfg.getThreads()), scheduler);

    for (int i = 1; i <= 20; ++i) {
        if (g_shutdownRequested.load()) {
            Logger::info("Stopping task submission due to shutdown request.");
            break;
        }

        pool.submit(Task(
            i,
            TaskPriority::MEDIUM,
            [i]() {
                std::cout << "[Phase 7] Task " << i
                          << " executing on thread "
                          << std::this_thread::get_id()
                          << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            },
            cfg.getMaxRetries()
        ));
    }

    Logger::info("Waiting for running tasks to complete...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Logger::info("Shutting down thread pool...");
    pool.shutdown();

    g_engineState.store(EngineState::TERMINATED);
}

// ---------------- PHASE 8: API MODE ----------------
void runApiMode() {
    Logger::info("===== PHASE 8: Production API Mode =====");
    
    Config& cfg = Config::instance();
    
    // Create scheduler
    std::shared_ptr<Scheduler> scheduler;
    if (cfg.getScheduler() == "priority") {
        scheduler = std::make_shared<PriorityScheduler>();
    } else {
        scheduler = std::make_shared<RoundRobinScheduler>();
    }
    
    // Create thread pool
    auto pool = std::make_shared<ThreadPool>(
        static_cast<size_t>(cfg.getThreads()), scheduler);
    
    // Start API server
    ApiServer apiServer(pool, cfg.getApiPort());
    apiServer.start();
    
    // Load tasks from JSON file if exists
    auto tasks = TaskLoader::loadFromJson("tasks.json");
    if (!tasks.empty()) {
        Logger::info("Loaded " + std::to_string(tasks.size()) + " tasks from tasks.json");
        for (const auto& def : tasks) {
            Task task = TaskLoader::createTask(def);
            TaskRegistry::instance().registerTask(task);
            pool->submit(task);
        }
    }
    
    Logger::info("API Server running. Press Ctrl+C to shutdown gracefully.");
    Logger::info("API Endpoints:");
    Logger::info("  GET  http://localhost:" + std::to_string(cfg.getApiPort()) + "/health");
    Logger::info("  GET  http://localhost:" + std::to_string(cfg.getApiPort()) + "/tasks");
    Logger::info("  GET  http://localhost:" + std::to_string(cfg.getApiPort()) + "/tasks/{id}");
    Logger::info("  POST http://localhost:" + std::to_string(cfg.getApiPort()) + "/tasks");
    
    // Wait for shutdown signal
    while (!g_shutdownRequested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    Logger::info("Shutting down API server and thread pool...");
    apiServer.stop();
    pool->shutdown();
    g_engineState.store(EngineState::TERMINATED);
}

// ---------------- MAIN ----------------
int main(int argc, char* argv[]) {
    // Initialize database and logging
    Database::instance().initialize("taskweave.db");
    Logger::initialize("taskweave.log", true);
    
    Logger::info("TaskWeave Engine Starting");

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Load config (order: defaults -> file -> environment -> args)
    Config& cfg = Config::instance();
    cfg.loadFromFile("src/config.ini");
    cfg.loadFromEnvironment();
    cfg.loadFromArgs(argc, argv);
    
    // Validate configuration
    if (!cfg.validate()) {
        Logger::error("Configuration validation failed. Some values may be incorrect.");
    }

    Logger::info(
        "Effective config: threads=" + std::to_string(cfg.getThreads()) +
        ", scheduler=" + cfg.getScheduler() +
        ", max_retries=" + std::to_string(cfg.getMaxRetries()) +
        ", mode=" + cfg.getMode() +
        ", api_port=" + std::to_string(cfg.getApiPort())
    );

    // Run in API mode or demo mode
    if (cfg.getMode() == "api") {
        runApiMode();
    } else {
        // Demo mode: run all phases
        runPhase1();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        runPhase2();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        runPhase3();
        runPhase7();
    }

    Logger::info("TaskWeave Engine Shutdown");
    Metrics::instance().printSummary();
    
    // Cleanup
    Database::instance().close();
    Logger::shutdown();

    return 0;
}
