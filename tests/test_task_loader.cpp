#include <gtest/gtest.h>
#include "../core/TaskLoader.h"
#include "../core/TaskDefinition.h"
#include "../src/core/Task.h"
#include <fstream>
#include <sstream>
#include <cstdio>

class TaskLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test JSON file
        testJsonFile = "test_tasks.json";
    }
    
    void TearDown() override {
        // Cleanup: remove test file if it exists
        std::remove(testJsonFile.c_str());
    }
    
    std::string testJsonFile;
};

// Test Load Tasks from Valid JSON String
TEST_F(TaskLoaderTest, LoadFromValidJsonString) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 1,
                "name": "Test Task",
                "priority": "HIGH",
                "max_retries": 2,
                "type": "print",
                "params": {
                    "message": "Hello World"
                }
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    ASSERT_EQ(tasks.size(), 1);
    
    EXPECT_EQ(tasks[0].id, 1);
    EXPECT_EQ(tasks[0].name, "Test Task");
    EXPECT_EQ(tasks[0].priority, "HIGH");
    EXPECT_EQ(tasks[0].maxRetries, 2);
    EXPECT_EQ(tasks[0].type, "print");
    EXPECT_EQ(tasks[0].params.at("message"), "Hello World");
}

// Test Load Multiple Tasks
TEST_F(TaskLoaderTest, LoadMultipleTasks) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 1,
                "name": "Task 1",
                "priority": "HIGH",
                "max_retries": 1,
                "type": "print"
            },
            {
                "id": 2,
                "name": "Task 2",
                "priority": "MEDIUM",
                "max_retries": 0,
                "type": "sleep"
            },
            {
                "id": 3,
                "name": "Task 3",
                "priority": "LOW",
                "max_retries": 3,
                "type": "print"
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    ASSERT_EQ(tasks.size(), 3);
    
    EXPECT_EQ(tasks[0].id, 1);
    EXPECT_EQ(tasks[0].priority, "HIGH");
    
    EXPECT_EQ(tasks[1].id, 2);
    EXPECT_EQ(tasks[1].priority, "MEDIUM");
    
    EXPECT_EQ(tasks[2].id, 3);
    EXPECT_EQ(tasks[2].priority, "LOW");
}

// Test Invalid JSON Handling
TEST_F(TaskLoaderTest, InvalidJsonHandling) {
    std::string invalidJson = "{ invalid json }";
    
    auto tasks = TaskLoader::loadFromJsonString(invalidJson);
    EXPECT_EQ(tasks.size(), 0);
}

// Test Missing Tasks Key
TEST_F(TaskLoaderTest, MissingTasksKey) {
    std::string jsonStr = R"({
        "other_key": "value"
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    EXPECT_EQ(tasks.size(), 0);
}

// Test Invalid Task ID
TEST_F(TaskLoaderTest, InvalidTaskId) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 0,
                "name": "Invalid ID",
                "priority": "MEDIUM",
                "max_retries": 0,
                "type": "print"
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    // Task with ID 0 should be filtered out
    EXPECT_EQ(tasks.size(), 0);
}

// Test Invalid Priority Defaults to MEDIUM
TEST_F(TaskLoaderTest, InvalidPriorityDefaultsToMedium) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 1,
                "name": "Test",
                "priority": "INVALID",
                "max_retries": 0,
                "type": "print"
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    ASSERT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].priority, "MEDIUM"); // Should default to MEDIUM
}

// Test Max Retries Validation
TEST_F(TaskLoaderTest, MaxRetriesValidation) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 1,
                "name": "Test",
                "priority": "HIGH",
                "max_retries": 150,
                "type": "print"
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    ASSERT_EQ(tasks.size(), 1);
    // max_retries > 100 should be clamped to 0
    EXPECT_EQ(tasks[0].maxRetries, 0);
}

// Test Missing Optional Fields
TEST_F(TaskLoaderTest, MissingOptionalFields) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 1,
                "name": "Minimal Task"
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    ASSERT_EQ(tasks.size(), 1);
    EXPECT_EQ(tasks[0].id, 1);
    EXPECT_EQ(tasks[0].name, "Minimal Task");
    // Default values should be applied
    EXPECT_EQ(tasks[0].priority, "MEDIUM");
    EXPECT_EQ(tasks[0].maxRetries, 0);
}

// Test Params Parsing
TEST_F(TaskLoaderTest, ParamsParsing) {
    std::string jsonStr = R"({
        "tasks": [
            {
                "id": 1,
                "name": "Task with params",
                "priority": "HIGH",
                "max_retries": 0,
                "type": "sleep",
                "params": {
                    "duration_ms": "500",
                    "message": "Test message"
                }
            }
        ]
    })";
    
    auto tasks = TaskLoader::loadFromJsonString(jsonStr);
    ASSERT_EQ(tasks.size(), 1);
    
    EXPECT_EQ(tasks[0].params.size(), 2);
    EXPECT_EQ(tasks[0].params.at("duration_ms"), "500");
    EXPECT_EQ(tasks[0].params.at("message"), "Test message");
}

// Test Create Task from Definition
TEST_F(TaskLoaderTest, CreateTaskFromDefinition) {
    TaskDefinition def;
    def.id = 100;
    def.name = "Test Task";
    def.priority = "HIGH";
    def.maxRetries = 2;
    def.type = "print";
    def.params["message"] = "Hello from test";
    
    Task task = TaskLoader::createTask(def);
    
    EXPECT_EQ(task.getId(), 100);
    EXPECT_EQ(task.getPriority(), TaskPriority::HIGH);
    EXPECT_EQ(task.getMaxRetries(), 2);
}

// Test Create Sleep Task
TEST_F(TaskLoaderTest, CreateSleepTask) {
    TaskDefinition def;
    def.id = 1;
    def.name = "Sleep Task";
    def.type = "sleep";
    def.params["duration_ms"] = "100";
    
    Task task = TaskLoader::createTask(def);
    EXPECT_EQ(task.getId(), 1);
    
    // Task should execute without throwing
    task.markReady();
    task.execute();
    EXPECT_EQ(task.getState(), TaskState::COMPLETED);
}

// Test Create Print Task
TEST_F(TaskLoaderTest, CreatePrintTask) {
    TaskDefinition def;
    def.id = 2;
    def.name = "Print Task";
    def.type = "print";
    def.params["message"] = "Test message";
    
    Task task = TaskLoader::createTask(def);
    EXPECT_EQ(task.getId(), 2);
    
    // Task should execute without throwing
    task.markReady();
    task.execute();
    EXPECT_EQ(task.getState(), TaskState::COMPLETED);
}
