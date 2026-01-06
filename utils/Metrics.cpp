#include "Metrics.h"

#include <iostream>

#include "Logger.h"

Metrics& Metrics::instance() {
    static Metrics instance;
    return instance;
}

void Metrics::recordTask(const Task& task) {
    using namespace std::chrono;

    const auto enqueueTime = task.getEnqueueTime();
    const auto startTime = task.getStartTime();
    const auto endTime = task.getEndTime();

    if (startTime == steady_clock::time_point() ||
        endTime == steady_clock::time_point() ||
        enqueueTime == steady_clock::time_point()) {
        return;
    }

    const auto waitTime = startTime - enqueueTime;
    const auto execTime = endTime - startTime;

    std::lock_guard<std::mutex> lock(mtx);

    ++totalTasks;
    totalRetries += static_cast<std::uint64_t>(task.getRetryCount());
    if (task.getState() == TaskState::COMPLETED) {
        ++completedTasks;
    } else if (task.getState() == TaskState::FAILED) {
        ++failedTasks;
        ++failedFinalTasks;
    }

    totalWaitTime += waitTime;
    totalExecTime += execTime;

    if (!hasExecSamples) {
        maxExecTime = minExecTime = execTime;
        hasExecSamples = true;
    } else {
        if (execTime > maxExecTime) maxExecTime = execTime;
        if (execTime < minExecTime) minExecTime = execTime;
    }
}

void Metrics::printSummary() const {
    using namespace std::chrono;

    std::lock_guard<std::mutex> lock(mtx);

    if (totalTasks == 0) {
        Logger::info("===== METRICS SUMMARY =====");
        Logger::info("No tasks were executed.");
        Logger::info("===========================");
        return;
    }

    const auto avgWaitMs =
        duration_cast<microseconds>(totalWaitTime).count() /
        (1000.0 * static_cast<double>(totalTasks));

    const auto avgExecMs =
        duration_cast<microseconds>(totalExecTime).count() /
        (1000.0 * static_cast<double>(totalTasks));

    const auto maxExecMs =
        hasExecSamples
            ? duration_cast<microseconds>(maxExecTime).count() / 1000.0
            : 0.0;

    const auto minExecMs =
        hasExecSamples
            ? duration_cast<microseconds>(minExecTime).count() / 1000.0
            : 0.0;

    std::cout << "===== METRICS SUMMARY =====\n";
    std::cout << "Tasks Executed   : " << totalTasks << "\n";
    std::cout << "Completed        : " << completedTasks << "\n";
    std::cout << "Failed           : " << failedTasks << "\n";
    std::cout << "Total Retries    : " << totalRetries << "\n\n";

    std::cout << "Avg Wait Time    : " << avgWaitMs << " ms\n";
    std::cout << "Avg Exec Time    : " << avgExecMs << " ms\n";
    std::cout << "Max Exec Time    : " << maxExecMs << " ms\n";
    std::cout << "Min Exec Time    : " << minExecMs << " ms\n";
    std::cout << "===========================\n";
}


