#include <gtest/gtest.h>
#include "../src/scheduler/PriorityScheduler.h"
#include "../src/scheduler/RoundRobinScheduler.h"
#include "../src/core/Task.h"
#include <thread>
#include <chrono>
#include <vector>

class SchedulerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }
    
    void TearDown() override {
        // Cleanup code if needed
    }
};

// ============================================================================
// PriorityScheduler Tests
// ============================================================================

// Test Priority Scheduler - Empty Check
TEST_F(SchedulerTest, PrioritySchedulerEmptyCheck) {
    PriorityScheduler scheduler;
    EXPECT_TRUE(scheduler.empty());
    
    Task task(1, TaskPriority::HIGH, []() {}, 0);
    task.markReady();
    scheduler.submit(task);
    
    EXPECT_FALSE(scheduler.empty());
}

// Test Priority Scheduler - High Priority First
TEST_F(SchedulerTest, PrioritySchedulerHighPriorityFirst) {
    PriorityScheduler scheduler;
    
    // Submit tasks in reverse priority order
    Task lowTask(1, TaskPriority::LOW, []() {}, 0);
    Task mediumTask(2, TaskPriority::MEDIUM, []() {}, 0);
    Task highTask(3, TaskPriority::HIGH, []() {}, 0);
    
    lowTask.markReady();
    mediumTask.markReady();
    highTask.markReady();
    
    scheduler.submit(lowTask);
    scheduler.submit(mediumTask);
    scheduler.submit(highTask);
    
    // Highest priority should come out first
    Task next = scheduler.getNextTask();
    EXPECT_EQ(next.getId(), 3); // High priority task
    EXPECT_EQ(next.getPriority(), TaskPriority::HIGH);
}

// Test Priority Scheduler - Order By Priority
TEST_F(SchedulerTest, PrioritySchedulerOrderByPriority) {
    PriorityScheduler scheduler;
    
    std::vector<Task> tasks;
    for (int i = 0; i < 3; i++) {
        TaskPriority priority = static_cast<TaskPriority>(i); // LOW, MEDIUM, HIGH
        Task task(i + 1, priority, []() {}, 0);
        task.markReady();
        tasks.push_back(task);
        scheduler.submit(task);
    }
    
    // Should retrieve in priority order: HIGH, MEDIUM, LOW
    Task first = scheduler.getNextTask();
    EXPECT_EQ(first.getPriority(), TaskPriority::HIGH);
    
    Task second = scheduler.getNextTask();
    EXPECT_EQ(second.getPriority(), TaskPriority::MEDIUM);
    
    Task third = scheduler.getNextTask();
    EXPECT_EQ(third.getPriority(), TaskPriority::LOW);
}

// Test Priority Scheduler - Same Priority FIFO
TEST_F(SchedulerTest, PrioritySchedulerSamePriorityFIFO) {
    PriorityScheduler scheduler;
    
    Task task1(1, TaskPriority::HIGH, []() {}, 0);
    Task task2(2, TaskPriority::HIGH, []() {}, 0);
    Task task3(3, TaskPriority::HIGH, []() {}, 0);
    
    task1.markReady();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    task2.markReady();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    task3.markReady();
    
    scheduler.submit(task1);
    scheduler.submit(task2);
    scheduler.submit(task3);
    
    // With same priority, earlier enqueued should come first
    Task first = scheduler.getNextTask();
    EXPECT_EQ(first.getId(), 1); // First submitted
}

// Test Priority Scheduler - Thread Safety
TEST_F(SchedulerTest, PrioritySchedulerThreadSafety) {
    PriorityScheduler scheduler;
    const int numTasks = 100;
    
    // Submit tasks from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&scheduler, i, numTasks]() {
            for (int j = 0; j < numTasks / 10; j++) {
                int id = i * (numTasks / 10) + j;
                TaskPriority priority = static_cast<TaskPriority>(id % 3);
                Task task(id, priority, []() {}, 0);
                task.markReady();
                scheduler.submit(task);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all tasks were submitted
    int count = 0;
    while (!scheduler.empty()) {
        scheduler.getNextTask();
        count++;
    }
    
    EXPECT_EQ(count, numTasks);
}

// ============================================================================
// RoundRobinScheduler Tests
// ============================================================================

// Test RoundRobin Scheduler - Empty Check
TEST_F(SchedulerTest, RoundRobinSchedulerEmptyCheck) {
    RoundRobinScheduler scheduler;
    EXPECT_TRUE(scheduler.empty());
    
    Task task(1, TaskPriority::MEDIUM, []() {}, 0);
    task.markReady();
    scheduler.submit(task);
    
    EXPECT_FALSE(scheduler.empty());
}

// Test RoundRobin Scheduler - FIFO Order
TEST_F(SchedulerTest, RoundRobinSchedulerFIFO) {
    RoundRobinScheduler scheduler;
    
    std::vector<int> taskIds;
    for (int i = 1; i <= 5; i++) {
        Task task(i, TaskPriority::MEDIUM, []() {}, 0);
        task.markReady();
        scheduler.submit(task);
        taskIds.push_back(i);
    }
    
    // Should retrieve in FIFO order
    for (int expectedId : taskIds) {
        EXPECT_FALSE(scheduler.empty());
        Task next = scheduler.getNextTask();
        EXPECT_EQ(next.getId(), expectedId);
    }
    
    EXPECT_TRUE(scheduler.empty());
}

// Test RoundRobin Scheduler - Order Preservation
TEST_F(SchedulerTest, RoundRobinSchedulerOrderPreservation) {
    RoundRobinScheduler scheduler;
    
    Task task1(1, TaskPriority::HIGH, []() {}, 0);
    Task task2(2, TaskPriority::LOW, []() {}, 0);
    Task task3(3, TaskPriority::MEDIUM, []() {}, 0);
    
    task1.markReady();
    task2.markReady();
    task3.markReady();
    
    scheduler.submit(task1);
    scheduler.submit(task2);
    scheduler.submit(task3);
    
    // RoundRobin ignores priority, maintains submission order
    Task first = scheduler.getNextTask();
    EXPECT_EQ(first.getId(), 1);
    
    Task second = scheduler.getNextTask();
    EXPECT_EQ(second.getId(), 2);
    
    Task third = scheduler.getNextTask();
    EXPECT_EQ(third.getId(), 3);
}

// Test RoundRobin Scheduler - Thread Safety
TEST_F(SchedulerTest, RoundRobinSchedulerThreadSafety) {
    RoundRobinScheduler scheduler;
    const int numTasks = 100;
    
    // Submit tasks from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&scheduler, i, numTasks]() {
            for (int j = 0; j < numTasks / 10; j++) {
                int id = i * (numTasks / 10) + j;
                Task task(id, TaskPriority::MEDIUM, []() {}, 0);
                task.markReady();
                scheduler.submit(task);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all tasks were submitted
    int count = 0;
    while (!scheduler.empty()) {
        scheduler.getNextTask();
        count++;
    }
    
    EXPECT_EQ(count, numTasks);
}

// Test RoundRobin vs Priority - Different Behavior
TEST_F(SchedulerTest, RoundRobinVsPriorityDifferentBehavior) {
    PriorityScheduler priorityScheduler;
    RoundRobinScheduler roundRobinScheduler;
    
    // Submit same tasks to both schedulers
    Task highTask(1, TaskPriority::HIGH, []() {}, 0);
    Task lowTask(2, TaskPriority::LOW, []() {}, 0);
    Task mediumTask(3, TaskPriority::MEDIUM, []() {}, 0);
    
    highTask.markReady();
    lowTask.markReady();
    mediumTask.markReady();
    
    priorityScheduler.submit(highTask);
    priorityScheduler.submit(lowTask);
    priorityScheduler.submit(mediumTask);
    
    roundRobinScheduler.submit(highTask);
    roundRobinScheduler.submit(lowTask);
    roundRobinScheduler.submit(mediumTask);
    
    // Priority scheduler should return HIGH first
    Task priorityFirst = priorityScheduler.getNextTask();
    EXPECT_EQ(priorityFirst.getPriority(), TaskPriority::HIGH);
    
    // RoundRobin scheduler should return in submission order (HIGH first because submitted first)
    Task roundRobinFirst = roundRobinScheduler.getNextTask();
    EXPECT_EQ(roundRobinFirst.getId(), 1); // First submitted
}
