#pragma once

enum class TaskState {
    CREATED,
    READY,
    RUNNING,
    RETRYING,
    COMPLETED,
    FAILED
};
