#include <iostream>
#include "core/Task.h"
#include "../utils/Logger.h"

int main() {
    Logger::info("TaskWeave Engine Starting (Phase 0)");

    Task task1(1, 5, []() {
        std::cout << "Executing Task 1\n";
    });

    Task task2(2, 1, []() {
        std::cout << "Executing Task 2\n";
    });

    task1.execute();
    task2.execute();

    Logger::info("All tasks executed successfully");
    return 0;
}
