#include <iostream>
#include <thread>

#include "core/Task.h"
#include "executor/ThreadPool.h"
#include "../utils/Logger.h"

int main() {
    Logger::info("TaskWeave Engine Starting (Phase 1)");

    ThreadPool pool(3);

    for (int i = 1; i <= 6; ++i) {
        pool.submit(Task(i, i, [i] {
            std::cout << "Executing Task " << i
                      << " on thread " << std::this_thread::get_id()
                      << std::endl;
        }));
    }

    Logger::info("All tasks submitted");
    return 0;
}
