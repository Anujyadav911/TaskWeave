#include "TaskRegistry.h"

TaskRegistry& TaskRegistry::instance() {
    static TaskRegistry registry;
    return registry;
}

void TaskRegistry::registerTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mtx);
    tasks[task.getId()] = std::make_shared<Task>(task);
}

std::shared_ptr<Task> TaskRegistry::getTask(int id) const {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = tasks.find(id);
    if (it != tasks.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Task>> TaskRegistry::getAllTasks() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::shared_ptr<Task>> result;
    for (const auto& pair : tasks) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<std::shared_ptr<Task>> TaskRegistry::getTasksByState(TaskState state) const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::shared_ptr<Task>> result;
    for (const auto& pair : tasks) {
        if (pair.second->getState() == state) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void TaskRegistry::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    tasks.clear();
}

size_t TaskRegistry::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return tasks.size();
}

