#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

// Forward declaration
struct sqlite3;
struct sqlite3_stmt;

// Task record for database
struct TaskRecord {
    int id;
    std::string name;
    std::string priority;
    int max_retries;
    int retry_count;
    int state;  // TaskState enum value
    std::string type;
    std::string params_json;
    std::string created_at;
    std::string started_at;
    std::string completed_at;
    std::string thread_id;
    std::string error_message;
};

class Database {
public:
    static Database& instance();
    
    // Initialize database connection
    bool initialize(const std::string& dbPath = "taskweave.db");
    
    // Close database connection
    void close();
    
    // Task operations
    bool createTask(const TaskRecord& task);
    bool updateTask(const TaskRecord& task);
    TaskRecord getTask(int id);
    std::vector<TaskRecord> getAllTasks();
    std::vector<TaskRecord> getTasksByState(int state);
    std::vector<TaskRecord> getTasksByDateRange(const std::string& startDate, const std::string& endDate);
    bool deleteTask(int id);
    bool deleteOldTasks(int daysOld);
    
    // Statistics
    int getTaskCount();
    int getTaskCountByState(int state);
    std::vector<std::pair<std::string, int>> getTaskStats();
    
    // Health check
    bool isConnected() const { return db != nullptr; }
    
    ~Database();

private:
    Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    sqlite3* db;
    mutable std::mutex mtx;
    
    bool executeQuery(const std::string& query);
    std::string getCurrentTimestamp();
};
