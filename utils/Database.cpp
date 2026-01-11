#include "Database.h"
#include "Logger.h"
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <ctime>

Database& Database::instance() {
    static Database db;
    return db;
}

Database::Database() : db(nullptr) {
}

bool Database::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (db != nullptr) {
        return true; // Already initialized
    }
    
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        Logger::error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
        sqlite3_close(db);
        db = nullptr;
        return false;
    }
    
    // Create tasks table
    std::string createTable = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            priority TEXT NOT NULL,
            max_retries INTEGER DEFAULT 0,
            retry_count INTEGER DEFAULT 0,
            state INTEGER NOT NULL,
            type TEXT,
            params_json TEXT,
            created_at TEXT NOT NULL,
            started_at TEXT,
            completed_at TEXT,
            thread_id TEXT,
            error_message TEXT
        );
        
        CREATE INDEX IF NOT EXISTS idx_state ON tasks(state);
        CREATE INDEX IF NOT EXISTS idx_created_at ON tasks(created_at);
    )";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, createTable.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        Logger::error("Failed to create table: " + std::string(errMsg));
        sqlite3_free(errMsg);
        sqlite3_close(db);
        db = nullptr;
        return false;
    }
    
    Logger::info("Database initialized: " + dbPath);
    return true;
}

void Database::close() {
    std::lock_guard<std::mutex> lock(mtx);
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

Database::~Database() {
    close();
}

std::string Database::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool Database::createTask(const TaskRecord& task) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!db) return false;
    
    std::string query = R"(
        INSERT INTO tasks (id, name, priority, max_retries, retry_count, state, type, params_json, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, task.id);
    sqlite3_bind_text(stmt, 2, task.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, task.priority.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, task.max_retries);
    sqlite3_bind_int(stmt, 5, task.retry_count);
    sqlite3_bind_int(stmt, 6, task.state);
    sqlite3_bind_text(stmt, 7, task.type.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, task.params_json.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, task.created_at.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::updateTask(const TaskRecord& task) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!db) return false;
    
    std::string query = R"(
        UPDATE tasks SET
            state = ?,
            retry_count = ?,
            started_at = ?,
            completed_at = ?,
            thread_id = ?,
            error_message = ?
        WHERE id = ?
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, task.state);
    sqlite3_bind_int(stmt, 2, task.retry_count);
    sqlite3_bind_text(stmt, 3, task.started_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, task.completed_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, task.thread_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, task.error_message.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, task.id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

TaskRecord Database::getTask(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    TaskRecord record;
    record.id = -1;
    
    if (!db) return record;
    
    std::string query = "SELECT * FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return record;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        record.id = sqlite3_column_int(stmt, 0);
        record.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        record.max_retries = sqlite3_column_int(stmt, 3);
        record.retry_count = sqlite3_column_int(stmt, 4);
        record.state = sqlite3_column_int(stmt, 5);
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        record.type = type ? type : "";
        const char* params = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        record.params_json = params ? params : "";
        const char* created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        record.created_at = created ? created : "";
        const char* started = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        record.started_at = started ? started : "";
        const char* completed = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        record.completed_at = completed ? completed : "";
        const char* thread = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        record.thread_id = thread ? thread : "";
        const char* error = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        record.error_message = error ? error : "";
    }
    
    sqlite3_finalize(stmt);
    return record;
}

std::vector<TaskRecord> Database::getAllTasks() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<TaskRecord> tasks;
    
    if (!db) return tasks;
    
    std::string query = "SELECT * FROM tasks ORDER BY created_at DESC LIMIT 1000";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return tasks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TaskRecord record;
        record.id = sqlite3_column_int(stmt, 0);
        record.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        record.max_retries = sqlite3_column_int(stmt, 3);
        record.retry_count = sqlite3_column_int(stmt, 4);
        record.state = sqlite3_column_int(stmt, 5);
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        record.type = type ? type : "";
        const char* params = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        record.params_json = params ? params : "";
        const char* created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        record.created_at = created ? created : "";
        const char* started = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        record.started_at = started ? started : "";
        const char* completed = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        record.completed_at = completed ? completed : "";
        const char* thread = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        record.thread_id = thread ? thread : "";
        const char* error = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        record.error_message = error ? error : "";
        tasks.push_back(record);
    }
    
    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<TaskRecord> Database::getTasksByState(int state) {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<TaskRecord> tasks;
    
    if (!db) return tasks;
    
    std::string query = "SELECT * FROM tasks WHERE state = ? ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return tasks;
    }
    
    sqlite3_bind_int(stmt, 1, state);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TaskRecord record;
        record.id = sqlite3_column_int(stmt, 0);
        record.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        record.max_retries = sqlite3_column_int(stmt, 3);
        record.retry_count = sqlite3_column_int(stmt, 4);
        record.state = sqlite3_column_int(stmt, 5);
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        record.type = type ? type : "";
        const char* params = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        record.params_json = params ? params : "";
        const char* created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        record.created_at = created ? created : "";
        const char* started = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        record.started_at = started ? started : "";
        const char* completed = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        record.completed_at = completed ? completed : "";
        const char* thread = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        record.thread_id = thread ? thread : "";
        const char* error = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        record.error_message = error ? error : "";
        tasks.push_back(record);
    }
    
    sqlite3_finalize(stmt);
    return tasks;
}

int Database::getTaskCount() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!db) return 0;
    
    std::string query = "SELECT COUNT(*) FROM tasks";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

int Database::getTaskCountByState(int state) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!db) return 0;
    
    std::string query = "SELECT COUNT(*) FROM tasks WHERE state = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_int(stmt, 1, state);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

bool Database::executeQuery(const std::string& query) {
    if (!db) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        Logger::error("Query failed: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

std::vector<TaskRecord> Database::getTasksByDateRange(const std::string& /* startDate */, const std::string& /* endDate */) {
    // Similar implementation to getAllTasks with date filtering
    return getAllTasks(); // Simplified for now
}

bool Database::deleteTask(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!db) return false;
    
    std::string query = "DELETE FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::deleteOldTasks(int daysOld) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!db) return false;
    
    std::string query = "DELETE FROM tasks WHERE created_at < datetime('now', '-' || ? || ' days')";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, daysOld);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<std::pair<std::string, int>> Database::getTaskStats() {
    std::vector<std::pair<std::string, int>> stats;
    stats.push_back({"total", getTaskCount()});
    stats.push_back({"pending", getTaskCountByState(0)});
    stats.push_back({"running", getTaskCountByState(2)});
    stats.push_back({"completed", getTaskCountByState(3)});
    stats.push_back({"failed", getTaskCountByState(4)});
    return stats;
}
