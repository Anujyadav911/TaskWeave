#pragma once

#include <map>
#include <mutex>
#include <memory>
#include "../src/core/Task.h"

// Registry to track all tasks (in-memory storage)
class TaskRegistry {
public:
    static TaskRegistry& instance();
    
    // Register a task
    void registerTask(const Task& task);
    
    // Get task by ID
    std::shared_ptr<Task> getTask(int id) const;
    
    // Get all tasks
    std::vector<std::shared_ptr<Task>> getAllTasks() const;
    
    // Get tasks by state
    std::vector<std::shared_ptr<Task>> getTasksByState(TaskState state) const;
    
    // Clear registry (for testing)
    void clear();
    
    size_t size() const;

private:
    TaskRegistry() = default;
    
    mutable std::mutex mtx;
    std::map<int, std::shared_ptr<Task>> tasks;
};

