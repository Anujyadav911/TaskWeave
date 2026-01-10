#pragma once

#include <string>
#include <map>
#include "../src/core/Task.h"

// Task definition from JSON
struct TaskDefinition {
    int id = 0;
    std::string name;
    std::string priority = "MEDIUM";  // LOW, MEDIUM, HIGH
    int maxRetries = 0;
    std::string type;  // "sleep", "print", "custom"
    std::map<std::string, std::string> params;  // task-specific parameters
    
    // Convert to TaskPriority enum
    TaskPriority getPriorityEnum() const {
        if (priority == "HIGH") return TaskPriority::HIGH;
        if (priority == "LOW") return TaskPriority::LOW;
        return TaskPriority::MEDIUM;
    }
};

