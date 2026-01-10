#include "TaskLoader.h"
#include "../src/core/Task.h"
#include "../utils/Logger.h"
#include "../third_party/json.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <chrono>

using json = nlohmann::json;

// Helper functions for JSON parsing (internal use only)
static TaskDefinition parseTaskJson(const json& taskJson);
static std::vector<TaskDefinition> loadFromJsonObject(const json& j);

TaskDefinition parseTaskJson(const json& taskJson) {
    TaskDefinition def;
    
    // Extract and validate id
    if (taskJson.contains("id") && taskJson["id"].is_number_integer()) {
        int id = taskJson["id"].get<int>();
        if (id > 0 && id < 2147483647) {
            def.id = id;
        } else {
            Logger::warn("Invalid task ID: " + std::to_string(id) + ". Must be between 1 and 2147483646");
            def.id = 0;
        }
    } else {
        Logger::warn("Task missing required 'id' field or invalid type");
        def.id = 0;
    }
    
    // Extract name
    if (taskJson.contains("name") && taskJson["name"].is_string()) {
        def.name = taskJson["name"].get<std::string>();
    }
    
    // Extract priority
    if (taskJson.contains("priority") && taskJson["priority"].is_string()) {
        std::string priority = taskJson["priority"].get<std::string>();
        // Validate priority value
        if (priority == "HIGH" || priority == "MEDIUM" || priority == "LOW") {
            def.priority = priority;
        } else {
            Logger::warn("Invalid priority value: " + priority + ". Using MEDIUM");
            def.priority = "MEDIUM";
        }
    }
    
    // Extract maxRetries (supports both "max_retries" and "maxRetries")
    if (taskJson.contains("max_retries") && taskJson["max_retries"].is_number_integer()) {
        int retries = taskJson["max_retries"].get<int>();
        if (retries >= 0 && retries <= 100) {
            def.maxRetries = retries;
        } else {
            Logger::warn("Invalid max_retries: " + std::to_string(retries) + ". Must be between 0 and 100");
            def.maxRetries = 0;
        }
    } else if (taskJson.contains("maxRetries") && taskJson["maxRetries"].is_number_integer()) {
        int retries = taskJson["maxRetries"].get<int>();
        if (retries >= 0 && retries <= 100) {
            def.maxRetries = retries;
        } else {
            Logger::warn("Invalid maxRetries: " + std::to_string(retries) + ". Must be between 0 and 100");
            def.maxRetries = 0;
        }
    }
    
    // Extract type
    if (taskJson.contains("type") && taskJson["type"].is_string()) {
        def.type = taskJson["type"].get<std::string>();
    }
    
    // Extract params
    if (taskJson.contains("params") && taskJson["params"].is_object()) {
        for (auto& [key, value] : taskJson["params"].items()) {
            if (value.is_string()) {
                def.params[key] = value.get<std::string>();
            } else if (value.is_number()) {
                def.params[key] = std::to_string(value.get<int>());
            } else if (value.is_boolean()) {
                def.params[key] = value.get<bool>() ? "true" : "false";
            }
        }
    }
    
    return def;
}

std::vector<TaskDefinition> TaskLoader::loadFromJson(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        Logger::error("Failed to open JSON file: " + jsonPath);
        return {};
    }
    
    try {
        json j;
        file >> j;
        return loadFromJsonObject(j);
    } catch (const json::parse_error& e) {
        Logger::error("JSON parse error in file " + jsonPath + ": " + std::string(e.what()));
        return {};
    } catch (const std::exception& e) {
        Logger::error("Error reading JSON file " + jsonPath + ": " + std::string(e.what()));
        return {};
    }
}

std::vector<TaskDefinition> TaskLoader::loadFromJsonString(const std::string& jsonStr) {
    try {
        json j = json::parse(jsonStr);
        return loadFromJsonObject(j);
    } catch (const json::parse_error& e) {
        Logger::error("JSON parse error: " + std::string(e.what()));
        return {};
    } catch (const std::exception& e) {
        Logger::error("Error parsing JSON string: " + std::string(e.what()));
        return {};
    }
}

std::vector<TaskDefinition> loadFromJsonObject(const json& j) {
    std::vector<TaskDefinition> tasks;
    
    // Check if "tasks" key exists
    if (!j.contains("tasks") || !j["tasks"].is_array()) {
        Logger::error("Invalid JSON: 'tasks' key not found or not an array");
        return tasks;
    }
    
    // Parse each task object
    for (const auto& taskJson : j["tasks"]) {
        try {
            TaskDefinition def = parseTaskJson(taskJson);
            if (def.id > 0) {
                tasks.push_back(def);
            }
        } catch (const std::exception& e) {
            Logger::warn("Failed to parse task: " + std::string(e.what()));
        }
    }
    
    return tasks;
}

Task TaskLoader::createTask(const TaskDefinition& def) {
    std::function<void()> fn;
    
    if (def.type == "sleep") {
        int duration = 100;  // default
        auto it = def.params.find("duration_ms");
        if (it != def.params.end()) {
            duration = std::stoi(it->second);
        }
        fn = [duration]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
        };
    } else if (def.type == "print") {
        std::string message = def.name;
        auto it = def.params.find("message");
        if (it != def.params.end()) {
            message = it->second;
        }
        fn = [message]() {
            std::cout << "[Task] " << message << std::endl;
        };
    } else {
        // Default: just print task name
        std::string name = def.name;
        fn = [name]() {
            std::cout << "[Task] Executing: " << name << std::endl;
        };
    }
    
    return Task(def.id, def.getPriorityEnum(), fn, def.maxRetries);
}

