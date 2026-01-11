// Define Windows version BEFORE any includes (required by cpp-httplib)
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00  // Windows 10 (required by httplib)
#endif
#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000000
#endif
#endif

#include "ApiServer.h"
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include "../core/TaskLoader.h"
#include "../core/TaskRegistry.h"
#include "../utils/Logger.h"
#include "../utils/Metrics.h"
#include "../utils/Config.h"
#include "../core/TaskDefinition.h"
#include "../src/core/Task.h"
#include <fstream>
#include <sstream>
#include <ctime>

using json = nlohmann::json;

// Disambiguate Logger: httplib defines Logger as a type alias, 
// but we want to use our Logger class from utils/Logger.h
// Using a type alias to explicitly refer to the global Logger class
using AppLogger = class Logger;

ApiServer::ApiServer(std::shared_ptr<ThreadPool> pool, int port)
    : threadPool(pool), port(port), running(false) {
    Config& cfg = Config::instance();
    maxRequestSize = cfg.getMaxRequestSize();
    corsOrigin = cfg.getCorsOrigin();
    server = std::make_unique<httplib::Server>();
}

ApiServer::~ApiServer() {
    stop();
}

void ApiServer::setCorsHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", corsOrigin);
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

void ApiServer::setupRoutes() {
    // CORS preflight handler
    server->Options(".*", [this](const httplib::Request& /* req */, httplib::Response& res) {
        setCorsHeaders(res);
        res.status = 200;
    });
    
    // Serve dashboard
    server->Get("/", [this](const httplib::Request& /* req */, httplib::Response& res) {
        std::ifstream file("web/dashboard.html");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            setCorsHeaders(res);
            res.set_content(buffer.str(), "text/html");
        } else {
            setCorsHeaders(res);
            res.status = 404;
        }
    });
    
    server->Get("/dashboard", [this](const httplib::Request& /* req */, httplib::Response& res) {
        std::ifstream file("web/dashboard.html");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            setCorsHeaders(res);
            res.set_content(buffer.str(), "text/html");
        } else {
            setCorsHeaders(res);
            res.status = 404;
        }
    });
    
    server->Get("/dashboard.html", [this](const httplib::Request& /* req */, httplib::Response& res) {
        std::ifstream file("web/dashboard.html");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            setCorsHeaders(res);
            res.set_content(buffer.str(), "text/html");
        } else {
            setCorsHeaders(res);
            res.status = 404;
        }
    });
    
    // Health check endpoint
    server->Get("/health", [this](const httplib::Request& /* req */, httplib::Response& res) {
        json healthJson = {
            {"status", "healthy"},
            {"engine", "running"},
            {"timestamp", std::time(nullptr)}
        };
        setCorsHeaders(res);
        res.set_content(healthJson.dump(), "application/json");
    });
    
    // Metrics endpoint
    server->Get("/metrics", [this](const httplib::Request& /* req */, httplib::Response& res) {
        auto tasks = TaskRegistry::instance().getAllTasks();
        
        int total = tasks.size();
        int running = 0, completed = 0, failed = 0, pending = 0;
        for (const auto& task : tasks) {
            int state = static_cast<int>(task->getState());
            if (state == 0) pending++;
            else if (state == 2) running++;
            else if (state == 3) completed++;
            else if (state == 4) failed++;
        }
        
        json metricsJson = {
            {"total_tasks", total},
            {"pending", pending},
            {"running", running},
            {"completed", completed},
            {"failed", failed},
            {"uptime_seconds", std::time(nullptr)},
            {"thread_pool_size", threadPool->getSize()}
        };
        setCorsHeaders(res);
        res.set_content(metricsJson.dump(), "application/json");
    });
    
    server->Get("/api/metrics", [this](const httplib::Request& /* req */, httplib::Response& res) {
        auto tasks = TaskRegistry::instance().getAllTasks();
        
        int total = tasks.size();
        int running = 0, completed = 0, failed = 0, pending = 0;
        for (const auto& task : tasks) {
            int state = static_cast<int>(task->getState());
            if (state == 0) pending++;
            else if (state == 2) running++;
            else if (state == 3) completed++;
            else if (state == 4) failed++;
        }
        
        json metricsJson = {
            {"total_tasks", total},
            {"pending", pending},
            {"running", running},
            {"completed", completed},
            {"failed", failed},
            {"uptime_seconds", std::time(nullptr)},
            {"thread_pool_size", threadPool->getSize()}
        };
        setCorsHeaders(res);
        res.set_content(metricsJson.dump(), "application/json");
    });
    
    // Get all tasks
    server->Get("/tasks", [this](const httplib::Request& /* req */, httplib::Response& res) {
        auto tasks = TaskRegistry::instance().getAllTasks();
        json tasksArray = json::array();
        
        for (const auto& task : tasks) {
            // Get priority string
            std::string priority = "MEDIUM";
            if (task->getPriority() == TaskPriority::HIGH) priority = "HIGH";
            else if (task->getPriority() == TaskPriority::LOW) priority = "LOW";
            
            json taskJson = {
                {"id", task->getId()},
                {"name", "Task " + std::to_string(task->getId())},
                {"priority", priority},
                {"state", static_cast<int>(task->getState())},
                {"retry_count", task->getRetryCount()},
                {"max_retries", task->getMaxRetries()},
                {"type", "print"},
                {"created_at", "2024-01-01 00:00:00"}
            };
            tasksArray.push_back(taskJson);
        }
        
        json responseJson = {{"tasks", tasksArray}};
        setCorsHeaders(res);
        res.set_content(responseJson.dump(), "application/json");
    });
    
    // Get task by ID
    server->Get(R"(/tasks/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            int taskId = std::stoi(req.matches[1]);
            auto task = TaskRegistry::instance().getTask(taskId);
            if (task) {
                json taskJson = {
                    {"id", task->getId()},
                    {"state", static_cast<int>(task->getState())},
                    {"retry_count", task->getRetryCount()},
                    {"max_retries", task->getMaxRetries()}
                };
                setCorsHeaders(res);
                res.set_content(taskJson.dump(), "application/json");
            } else {
                setCorsHeaders(res);
                res.status = 404;
                json errorJson = {{"error", "Task not found"}};
                res.set_content(errorJson.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            setCorsHeaders(res);
            res.status = 400;
            json errorJson = {{"error", "Invalid task ID"}};
            res.set_content(errorJson.dump(), "application/json");
        }
    });
    
    // Submit task
    server->Post("/tasks", [this](const httplib::Request& req, httplib::Response& res) {
        // Check request size
        if (req.body.size() > static_cast<size_t>(maxRequestSize)) {
            setCorsHeaders(res);
            res.status = 413;
            json errorJson = {{"error", "Request entity too large"}};
            res.set_content(errorJson.dump(), "application/json");
            return;
        }
        
        try {
            AppLogger::info("Received POST /tasks with body: " + req.body);
            auto defs = TaskLoader::loadFromJsonString(req.body);
            
            if (!defs.empty()) {
                auto def = defs[0];
                
                // Validate task ID is not already registered
                if (TaskRegistry::instance().getTask(def.id) != nullptr) {
                    AppLogger::warn("Task ID " + std::to_string(def.id) + " already exists");
                    setCorsHeaders(res);
                    res.status = 409;
                    json errorJson = {{"error", "Task ID already exists"}};
                    res.set_content(errorJson.dump(), "application/json");
                    return;
                }
                
                Task task = TaskLoader::createTask(def);
                TaskRegistry::instance().registerTask(task);
                threadPool->submit(task);
                
                json successJson = {
                    {"status", "submitted"},
                    {"task_id", def.id}
                };
                setCorsHeaders(res);
                res.set_content(successJson.dump(), "application/json");
                AppLogger::info("Task " + std::to_string(def.id) + " submitted successfully");
            } else {
                AppLogger::error("Failed to parse task from JSON body");
                setCorsHeaders(res);
                res.status = 400;
                json errorJson = {{"error", "Invalid task format"}};
                res.set_content(errorJson.dump(), "application/json");
            }
        } catch (const json::parse_error& e) {
            AppLogger::error("JSON parse error in POST /tasks: " + std::string(e.what()));
            setCorsHeaders(res);
            res.status = 400;
            json errorJson = {{"error", "Invalid JSON format"}};
            res.set_content(errorJson.dump(), "application/json");
        } catch (const std::exception& e) {
            AppLogger::error("Exception in POST /tasks: " + std::string(e.what()));
            setCorsHeaders(res);
            res.status = 400;
            json errorJson = {{"error", e.what()}};
            res.set_content(errorJson.dump(), "application/json");
        } catch (...) {
            AppLogger::error("Unknown exception in POST /tasks");
            setCorsHeaders(res);
            res.status = 500;
            json errorJson = {{"error", "Failed to process request"}};
            res.set_content(errorJson.dump(), "application/json");
        }
    });
    
    // 404 handler for unmatched routes
    server->set_error_handler([](const httplib::Request& /* req */, httplib::Response& res) {
        res.status = 404;
        json errorJson = {{"error", "Not found"}};
        res.set_content(errorJson.dump(), "application/json");
    });
}

void ApiServer::start() {
    if (running.load()) {
        AppLogger::warn("API Server already running");
        return;
    }
    
    setupRoutes();
    
    running = true;
    serverThread = std::thread([this]() {
        AppLogger::info("API Server started on port " + std::to_string(port));
        if (!server->listen("0.0.0.0", port)) {
            AppLogger::error("Failed to start API Server on port " + std::to_string(port));
            running = false;
        }
    });
    
    // Give server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void ApiServer::stop() {
    if (!running.load()) {
        return;
    }
    
    running = false;
    
    // Stop the server
    if (server) {
        server->stop();
    }
    
    // Wait for server thread
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    AppLogger::info("API Server stopped");
}
