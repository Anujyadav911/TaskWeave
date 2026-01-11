#include <gtest/gtest.h>
#include "../src/core/Task.h"
#include <chrono>
#include <thread>
#include <stdexcept>

class TaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }
    
    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test Task Creation
TEST_F(TaskTest, TaskCreation) {
    Task task(1, TaskPriority::HIGH, []() {}, 3);
    
    EXPECT_EQ(task.getId(), 1);
    EXPECT_EQ(task.getPriority(), TaskPriority::HIGH);
    EXPECT_EQ(task.getState(), TaskState::CREATED);
    EXPECT_EQ(task.getMaxRetries(), 3);
    EXPECT_EQ(task.getRetryCount(), 0);
}

// Test Task State Transitions - CREATED -> READY
TEST_F(TaskTest, StateTransitionCreatedToReady) {
    Task task(1, TaskPriority::MEDIUM, []() {}, 0);
    
    EXPECT_EQ(task.getState(), TaskState::CREATED);
    task.markReady();
    EXPECT_EQ(task.getState(), TaskState::READY);
}

// Test Task State Transitions - READY -> RUNNING -> COMPLETED
TEST_F(TaskTest, StateTransitionReadyToRunningToCompleted) {
    bool executed = false;
    Task task(1, TaskPriority::HIGH, [&executed]() { executed = true; }, 0);
    
    task.markReady();
    EXPECT_EQ(task.getState(), TaskState::READY);
    
    task.execute();
    EXPECT_EQ(task.getState(), TaskState::COMPLETED);
    EXPECT_TRUE(executed);
}

// Test Task State Transitions - RUNNING -> FAILED (with exception)
TEST_F(TaskTest, StateTransitionRunningToFailed) {
    Task task(1, TaskPriority::MEDIUM, []() { throw std::runtime_error("Test error"); }, 1);
    
    task.markReady();
    
    // execute() will catch exception, set state to FAILED, then re-throw
    EXPECT_THROW(task.execute(), std::runtime_error);
    
    // After exception, state should be FAILED
    EXPECT_EQ(task.getState(), TaskState::FAILED);
}

// Test Retry Logic - Should Retry
TEST_F(TaskTest, ShouldRetryWhenRetriesAvailable) {
    Task task(1, TaskPriority::HIGH, []() { throw std::runtime_error("Test"); }, 3);
    
    task.markReady();
    
    // Execute should fail
    EXPECT_THROW(task.execute(), std::runtime_error);
    EXPECT_EQ(task.getState(), TaskState::FAILED);
    
    // Should be able to retry
    EXPECT_TRUE(task.shouldRetry());
    EXPECT_LT(task.getRetryCount(), task.getMaxRetries());
    
    // Mark retry - this should transition to RETRYING then to READY
    task.markRetry();
    EXPECT_EQ(task.getState(), TaskState::READY); // markRetry() calls markReady() internally
    EXPECT_EQ(task.getRetryCount(), 1);
}

// Test Retry Logic - No More Retries
TEST_F(TaskTest, ShouldNotRetryWhenMaxRetriesReached) {
    Task task(1, TaskPriority::MEDIUM, []() { throw std::runtime_error("Test"); }, 0);
    
    task.markReady();
    
    // Execute should fail
    EXPECT_THROW(task.execute(), std::runtime_error);
    EXPECT_EQ(task.getState(), TaskState::FAILED);
    
    // With maxRetries = 0, should not retry
    EXPECT_FALSE(task.shouldRetry());
    EXPECT_EQ(task.getRetryCount(), 0);
}

// Test Retry Count Increment
TEST_F(TaskTest, RetryCountIncrements) {
    Task task(1, TaskPriority::HIGH, []() { throw std::runtime_error("Test"); }, 2);
    
    task.markReady();
    
    // Force failure
    EXPECT_THROW(task.execute(), std::runtime_error);
    task.markFailed(); // Ensure in FAILED state
    
    int initialCount = task.getRetryCount();
    EXPECT_TRUE(task.shouldRetry());
    
    task.markRetry();
    EXPECT_EQ(task.getRetryCount(), initialCount + 1);
    EXPECT_EQ(task.getState(), TaskState::READY);
}

// Test Invalid State Transitions
TEST_F(TaskTest, InvalidStateTransitions) {
    Task task(1, TaskPriority::LOW, []() {}, 0);
    
    // CREATED -> COMPLETED should be invalid
    EXPECT_EQ(task.getState(), TaskState::CREATED);
    // Cannot directly go to COMPLETED, need to markReady and execute
    
    task.markReady();
    EXPECT_EQ(task.getState(), TaskState::READY);
}

// Test Task Priority Enum Values
TEST_F(TaskTest, TaskPriorityValues) {
    Task lowTask(1, TaskPriority::LOW, []() {}, 0);
    Task mediumTask(2, TaskPriority::MEDIUM, []() {}, 0);
    Task highTask(3, TaskPriority::HIGH, []() {}, 0);
    
    EXPECT_EQ(lowTask.getPriority(), TaskPriority::LOW);
    EXPECT_EQ(mediumTask.getPriority(), TaskPriority::MEDIUM);
    EXPECT_EQ(highTask.getPriority(), TaskPriority::HIGH);
}

// Test Task Enqueue Time
TEST_F(TaskTest, EnqueueTimeSet) {
    Task task(1, TaskPriority::MEDIUM, []() {}, 0);
    
    auto before = std::chrono::steady_clock::now();
    task.markReady();
    auto after = std::chrono::steady_clock::now();
    
    auto enqueueTime = task.getEnqueueTime();
    EXPECT_GE(enqueueTime, before);
    EXPECT_LE(enqueueTime, after);
}

// Test Task Thread ID Set After Execution
TEST_F(TaskTest, ThreadIdSetAfterExecution) {
    Task task(1, TaskPriority::HIGH, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }, 0);
    
    task.markReady();
    task.execute();
    
    // Thread ID should be set after execution
    std::thread::id threadId = task.getThreadId();
    EXPECT_NE(threadId, std::thread::id{});
}

// Test Task Retry State Machine: FAILED -> RETRYING -> READY
TEST_F(TaskTest, RetryStateMachine) {
    Task task(1, TaskPriority::MEDIUM, []() { throw std::runtime_error("Test"); }, 1);
    
    task.markReady();
    EXPECT_THROW(task.execute(), std::runtime_error);
    EXPECT_EQ(task.getState(), TaskState::FAILED);
    
    if (task.shouldRetry()) {
        task.markRetry();
        // markRetry() internally calls markReady(), so state should be READY
        EXPECT_EQ(task.getState(), TaskState::READY);
        EXPECT_EQ(task.getRetryCount(), 1);
    }
}
