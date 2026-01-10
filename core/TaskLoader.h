#pragma once

#include <vector>
#include <string>
#include "TaskDefinition.h"

class TaskLoader {
public:
    // Load tasks from JSON file
    static std::vector<TaskDefinition> loadFromJson(const std::string& jsonPath);
    
    // Load tasks from JSON string (for API)
    static std::vector<TaskDefinition> loadFromJsonString(const std::string& jsonStr);
    
    // Convert TaskDefinition to executable Task
    static Task createTask(const TaskDefinition& def);
};

