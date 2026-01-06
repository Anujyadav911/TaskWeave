#pragma once

#include <mutex>
#include <chrono>

#include "../src/core/Task.h"

class Metrics {
public:
    static Metrics& instance();

    void recordTask(const Task& task);
    void printSummary() const;

private:
    Metrics() = default;

    mutable std::mutex mtx;

    std::uint64_t totalTasks = 0;
    std::uint64_t completedTasks = 0;
    std::uint64_t failedTasks = 0;

    std::chrono::steady_clock::duration totalWaitTime{};
    std::chrono::steady_clock::duration totalExecTime{};

    std::chrono::steady_clock::duration maxExecTime{};
    std::chrono::steady_clock::duration minExecTime{};
    bool hasExecSamples = false;
};


