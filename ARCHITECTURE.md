# 🏛️ TaskWeave Architecture Documentation

Comprehensive architecture documentation covering design decisions, patterns, and implementation details.

---

## 📋 Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Design Principles](#design-principles)
3. [Design Patterns](#design-patterns)
4. [Core Components](#core-components)
5. [Task Lifecycle Management](#task-lifecycle-management)
6. [Thread Safety & Concurrency](#thread-safety--concurrency)
7. [Scheduling Algorithms](#scheduling-algorithms)
8. [Configuration Management](#configuration-management)
9. [Error Handling & Resilience](#error-handling--resilience)
10. [Performance Considerations](#performance-considerations)
11. [Scalability & Limitations](#scalability--limitations)
12. [Technology Stack](#technology-stack)
13. [Trade-offs & Design Decisions](#trade-offs--design-decisions)
14. [Future Improvements](#future-improvements)

---

## 🎯 Architecture Overview

TaskWeave follows a **layered architecture** with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│                    Presentation Layer                    │
│              (REST API, Web Dashboard)                   │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                    Application Layer                     │
│        (Task Registry, Task Loader, Configuration)      │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                     Domain Layer                         │
│              (Task, State Machine, Scheduler)            │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                  Infrastructure Layer                    │
│        (Thread Pool, Logger, Metrics, Database)         │
└─────────────────────────────────────────────────────────┘
```

### Component Interaction Diagram

```
Client Request
    │
    ▼
┌─────────────┐
│ API Server  │
└──────┬──────┘
       │
       ├─────────────────┐
       │                 │
       ▼                 ▼
┌──────────────┐  ┌──────────────┐
│Task Registry │  │ Task Loader  │
└──────┬───────┘  └──────┬───────┘
       │                 │
       │                 │ Creates
       │                 ▼
       │          ┌──────────────┐
       │          │    Task      │
       │          └──────┬───────┘
       │                 │
       │                 │ Submit
       ▼                 ▼
┌──────────────┐  ┌──────────────┐
│  Scheduler   │◄─┤ Thread Pool  │
│  (Strategy)  │  │  (Executor)  │
└──────────────┘  └──────┬───────┘
                         │
                         │ Execute
                         ▼
                   ┌──────────────┐
                   │   Metrics    │
                   └──────────────┘
```

---

## 🎨 Design Principles

### 1. **Separation of Concerns**

Each component has a single, well-defined responsibility:
- **Task**: Encapsulates task data and lifecycle
- **Scheduler**: Decides execution order
- **ThreadPool**: Manages execution threads
- **TaskRegistry**: Stores and retrieves tasks
- **ApiServer**: Handles HTTP communication

### 2. **Open/Closed Principle**

The system is open for extension, closed for modification:
- **Schedulers**: New scheduling algorithms can be added by implementing the `Scheduler` interface
- **Task Types**: New task types can be added by extending `TaskLoader`
- **Configuration**: New configuration options can be added without changing core logic

### 3. **Dependency Inversion**

High-level modules depend on abstractions:
- `ThreadPool` depends on `Scheduler` interface, not concrete implementations
- Components depend on `Config` interface, not configuration details

### 4. **Single Responsibility Principle**

Each class has one reason to change:
- `Task`: Task lifecycle only
- `PriorityScheduler`: Priority-based scheduling only
- `ThreadPool`: Thread management only

### 5. **Thread Safety by Design**

All shared data structures are protected by mutexes or use atomic operations.

---

## 🎭 Design Patterns

### 1. **Strategy Pattern** - Schedulers

**Purpose**: Allow runtime selection of scheduling algorithms.

**Implementation**:
```cpp
class Scheduler {  // Strategy interface
    virtual Task getNextTask() = 0;
};

class PriorityScheduler : public Scheduler { ... };
class RoundRobinScheduler : public Scheduler { ... };
```

**Benefits**:
- Easy to add new scheduling algorithms
- Runtime selection based on configuration
- No modification to `ThreadPool` when adding schedulers

**Usage**:
```cpp
std::shared_ptr<Scheduler> scheduler;
if (config.getScheduler() == "priority") {
    scheduler = std::make_shared<PriorityScheduler>();
} else {
    scheduler = std::make_shared<RoundRobinScheduler>();
}
ThreadPool pool(threadCount, scheduler);
```

---

### 2. **Singleton Pattern** - Configuration & Registry

**Purpose**: Ensure single instance of shared resources.

**Implementation** (Meyer's Singleton):
```cpp
class Config {
public:
    static Config& instance() {
        static Config cfg;  // Thread-safe in C++11+
        return cfg;
    }
private:
    Config() = default;  // Private constructor
};
```

**Used In**:
- `Config`: Single configuration instance
- `TaskRegistry`: Single task storage
- `Metrics`: Single metrics collector
- `Database`: Single database connection

**Benefits**:
- Global access point
- Thread-safe initialization (C++11 guarantee)
- Lazy initialization

**Trade-offs**:
- Makes testing harder (can be mitigated with dependency injection)
- Global state can be problematic in complex systems

---

### 3. **Factory Pattern** - Task Creation

**Purpose**: Create tasks from JSON definitions.

**Implementation**:
```cpp
class TaskLoader {
public:
    static Task createTask(const TaskDefinition& def);
    static std::vector<TaskDefinition> loadFromJson(const std::string& path);
};
```

**Benefits**:
- Centralized task creation logic
- Hides complexity of task initialization
- Easy to extend for new task types

---

### 4. **State Machine Pattern** - Task Lifecycle

**Purpose**: Model task state transitions explicitly.

**Implementation**:
```cpp
enum class TaskState {
    CREATED, READY, RUNNING, RETRYING, COMPLETED, FAILED
};

class Task {
    bool canTransition(TaskState from, TaskState to) const;
    TaskState state;
};
```

**Benefits**:
- Prevents invalid state transitions
- Clear lifecycle model
- Easy to reason about task state

**State Transition Rules**:
- `CREATED → READY`: Task is queued
- `READY → RUNNING`: Thread picks up task
- `RUNNING → COMPLETED`: Execution succeeds
- `RUNNING → FAILED`: Execution throws exception
- `FAILED → RETRYING`: Retry is scheduled
- `RETRYING → READY`: Task is re-queued

---

### 5. **Observer Pattern** (Implicit) - Metrics

**Purpose**: Track task execution for metrics.

**Implementation**:
- `Metrics::recordTask()` is called after task execution
- Collects statistics without tight coupling

---

## 🧩 Core Components

### 1. Task

**Responsibility**: Encapsulate task data and lifecycle.

**Key Features**:
- Immutable ID and priority (set at construction)
- Mutable state (controlled transitions)
- Function to execute (`std::function<void()>`)
- Retry tracking
- Timing information (enqueue, start, end times)

**Design Decisions**:
- **State Machine**: Prevents invalid transitions
- **Function Object**: Flexible task execution (lambda, function pointer, functor)
- **Immutable Core Data**: ID and priority don't change, ensuring consistency
- **Timing Fields**: Enable performance analysis

**Thread Safety**: Individual `Task` objects are not thread-safe. Thread safety is handled at the `ThreadPool` and `TaskRegistry` level.

---

### 2. ThreadPool

**Responsibility**: Manage worker threads and execute tasks.

**Key Features**:
- Fixed-size thread pool (configurable)
- Task queue (via Scheduler)
- Graceful shutdown (finish queued tasks)
- Force shutdown (immediate stop)

**Design Decisions**:
- **Fixed Size**: Prevents thread explosion, predictable resource usage
- **Scheduler Integration**: Pluggable scheduling via Strategy pattern
- **Condition Variables**: Efficient thread synchronization (threads sleep when no work)
- **Graceful Shutdown**: Ensures tasks complete, prevents data loss

**Thread Safety**: Uses mutex and condition variables for synchronization.

**Worker Thread Lifecycle**:
```
Start → Wait for task → Execute → Update metrics → Repeat
                          │
                          └── (Shutdown signal) → Cleanup → Exit
```

---

### 3. Scheduler

**Responsibility**: Determine task execution order.

**Implementations**:

#### PriorityScheduler
- Uses `std::priority_queue`
- Orders by: Priority (HIGH > MEDIUM > LOW), then enqueue time (FIFO)
- **Time Complexity**: O(log n) insert, O(log n) extract
- **Space Complexity**: O(n)

#### RoundRobinScheduler
- Uses `std::queue`
- FIFO ordering (fair distribution)
- **Time Complexity**: O(1) insert, O(1) extract
- **Space Complexity**: O(n)

**Design Decisions**:
- **Interface-Based**: Easy to swap implementations
- **Thread-Safe**: Each scheduler protects its queue with mutex
- **Simple Data Structures**: Standard library containers for reliability

---

### 4. TaskRegistry

**Responsibility**: Store and retrieve task instances.

**Key Features**:
- In-memory storage (`std::map<int, std::shared_ptr<Task>>`)
- Thread-safe access
- Query by ID or state

**Design Decisions**:
- **Shared Pointers**: Multiple components can reference tasks safely
- **Map Structure**: O(log n) lookup by ID
- **In-Memory Only**: Fast access, but data lost on restart (can be extended with persistence)

**Trade-offs**:
- **Pros**: Fast, simple, no I/O overhead
- **Cons**: Not persistent, limited by memory

---

### 5. TaskLoader

**Responsibility**: Parse JSON and create Task objects.

**Key Features**:
- JSON parsing (using nlohmann/json)
- Task type resolution
- Parameter extraction

**Design Decisions**:
- **Static Methods**: No state needed, simple interface
- **JSON Library**: nlohmann/json for robust parsing
- **Task Type System**: Extensible for new task types

---

### 6. ApiServer

**Responsibility**: Handle HTTP requests and route to appropriate handlers.

**Key Features**:
- REST endpoints (GET, POST)
- JSON request/response
- CORS support
- Error handling

**Design Decisions**:
- **cpp-httplib**: Header-only library, easy integration
- **Separate Thread**: API server runs in own thread, doesn't block main execution
- **CORS Support**: Enables web dashboard integration

---

## 🔄 Task Lifecycle Management

### State Machine Design

Tasks follow a strict state machine to ensure correctness:

```
                    ┌─────────┐
                    │ CREATED │
                    └────┬────┘
                         │ markReady()
                         ▼
                    ┌─────────┐
                    │  READY  │◄──────────┐
                    └────┬────┘           │
                         │ execute()      │ markRetry()
                         ▼                │
                    ┌─────────┐          │
                    │ RUNNING │          │
                    └────┬────┘          │
                         │               │
            ┌────────────┴────────────┐  │
            │                         │  │
    (Success)                    (Exception)
            │                         │
            ▼                         ▼
     ┌───────────┐             ┌───────────┐
     │ COMPLETED │             │  FAILED   │
     └───────────┘             └─────┬─────┘
                                     │
                            (shouldRetry() && retries < max)
                                     │
                                     ▼
                              ┌───────────┐
                              │ RETRYING  │
                              └───────────┘
```

### State Transition Rules

**Enforced by `canTransition()` method**:

| From State | To State | Condition |
|------------|----------|-----------|
| CREATED | READY | Always allowed (initial queue) |
| READY | RUNNING | Thread picks up task |
| RUNNING | COMPLETED | Execution succeeds (no exception) |
| RUNNING | FAILED | Execution throws exception |
| FAILED | RETRYING | `shouldRetry()` returns true |
| RETRYING | READY | Retry scheduled (moves to queue) |

**Invalid Transitions**: Blocked by `canTransition()` check, ensuring state consistency.

### Retry Mechanism

1. Task execution throws exception → state = FAILED
2. `shouldRetry()` checks: `retryCount < maxRetries`
3. If true: `markRetry()` → state = RETRYING → state = READY
4. Task re-enters scheduler queue
5. If false: Task remains in FAILED state

**Design Decision**: Retries are immediate (no exponential backoff). This can be enhanced in future versions.

---

## 🔒 Thread Safety & Concurrency

### Thread Safety Strategy

**1. Mutex Protection**
- All shared data structures protected by `std::mutex`
- Lock guards for automatic unlock: `std::lock_guard<std::mutex>`

**2. Atomic Operations**
- Engine state: `std::atomic<EngineState>`
- Shutdown flag: `std::atomic<bool>`

**3. Thread-Safe Singletons**
- C++11 guarantees thread-safe initialization of function-local statics
- No explicit synchronization needed for singleton creation

### Critical Sections

**TaskRegistry**:
```cpp
void registerTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mtx);  // Protects tasks map
    tasks[task.getId()] = std::make_shared<Task>(task);
}
```

**Scheduler** (PriorityScheduler example):
```cpp
Task getNextTask() override {
    std::lock_guard<std::mutex> lock(mtx);  // Protects priority queue
    // ... extract task ...
}
```

**ThreadPool**:
- Mutex protects scheduler access
- Condition variable coordinates thread wakeup
- Atomic flags for shutdown coordination

### Concurrency Model

**Producer-Consumer Pattern**:
- **Producers**: API server, task loader (submit tasks)
- **Consumers**: Worker threads (execute tasks)
- **Queue**: Scheduler (thread-safe queue)

**Synchronization**:
- Condition variables: Workers sleep when queue is empty
- Mutexes: Protect shared data structures
- Atomic flags: Coordinate shutdown

---

## 📊 Scheduling Algorithms

### Priority Scheduler

**Algorithm**: Priority Queue (Max-Heap)

**Ordering**:
1. **Primary**: Priority (HIGH > MEDIUM > LOW)
2. **Secondary**: Enqueue time (FIFO for same priority)

**Implementation**:
```cpp
struct TaskComparator {
    bool operator()(const Task& a, const Task& b) const {
        if (a.getPriority() != b.getPriority())
            return static_cast<int>(a.getPriority()) < 
                   static_cast<int>(b.getPriority());
        return a.getEnqueueTime() > b.getEnqueueTime();
    }
};
std::priority_queue<Task, std::vector<Task>, TaskComparator> pq;
```

**Use Cases**:
- Real-time systems
- Priority-based workloads
- Critical tasks first

**Trade-offs**:
- **Pros**: Urgent tasks execute first
- **Cons**: Low-priority tasks may starve

---

### Round-Robin Scheduler

**Algorithm**: FIFO Queue

**Ordering**: First-in, first-out (fair distribution)

**Implementation**:
```cpp
std::queue<Task> queue;
```

**Use Cases**:
- Fair task distribution
- Equal priority workloads
- Time-sharing systems

**Trade-offs**:
- **Pros**: No starvation, predictable ordering
- **Cons**: Urgent tasks may wait

---

## ⚙️ Configuration Management

### Configuration Hierarchy

Configuration is loaded in this order (later sources override earlier):

1. **Defaults** (hardcoded in `Config` class)
2. **Configuration File** (`config.ini`)
3. **Environment Variables** (`TASKWEAVE_*`)
4. **Command-Line Arguments** (highest priority)

### Configuration Sources

**File (`config.ini`)**:
```ini
threads=4
scheduler=priority
max_retries=2
api_port=8080
mode=api
```

**Environment Variables**:
```bash
export TASKWEAVE_THREADS=8
export TASKWEAVE_SCHEDULER=priority
export TASKWEAVE_API_PORT=8080
```

**Command-Line Arguments**:
```bash
./taskweave --threads=8 --scheduler=roundrobin --api-port=9090
```

### Design Decisions

- **Singleton Pattern**: Single configuration instance
- **Validation**: Configuration values validated on load
- **Immutable After Load**: Configuration doesn't change at runtime (simplifies design)
- **Multi-Source**: Flexible deployment (dev/test/prod)

---

## 🛡️ Error Handling & Resilience

### Error Handling Strategy

**1. Exception-Based Error Handling**
- Tasks throw exceptions on failure
- Exceptions caught at thread pool level
- Task state set to FAILED

**2. Retry Mechanism**
- Automatic retry for failed tasks
- Configurable maximum retries
- Retry count tracked per task

**3. Graceful Degradation**
- API server continues even if some tasks fail
- Metrics track failures
- Logging captures errors

### Resilience Features

**Graceful Shutdown**:
- SIGINT/SIGTERM handlers
- Engine state transitions: RUNNING → SHUTTING_DOWN → TERMINATED
- Thread pool finishes queued tasks before shutdown

**Resource Management**:
- RAII: Automatic resource cleanup
- Smart pointers: Automatic memory management
- Mutex locks: Automatic unlock via lock guards

**Error Recovery**:
- Task retries
- State machine prevents invalid states
- Logging for debugging

---

## ⚡ Performance Considerations

### Optimization Strategies

**1. Thread Pool**
- Reuses threads (avoids creation overhead)
- Fixed size (predictable resource usage)
- Worker threads sleep when idle (low CPU)

**2. Data Structures**
- Priority queue: O(log n) operations
- Map: O(log n) lookups
- Queue: O(1) operations

**3. Memory Management**
- Shared pointers: Automatic cleanup
- Move semantics: Avoid unnecessary copies
- Stack allocation where possible

**4. Synchronization**
- Fine-grained locking (per-component mutexes)
- Condition variables: Efficient thread wakeup
- Atomic operations: Lock-free where possible

### Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| Task Submission | O(log n) | Priority queue insert |
| Task Retrieval | O(log n) | Priority queue extract |
| Task Lookup (by ID) | O(log n) | Map lookup |
| Task Execution | O(1) | User-defined function |
| Thread Creation | O(1) | Fixed pool size |

**Bottlenecks**:
- Scheduler mutex contention (high task submission rate)
- Task execution time (user-defined function)
- Thread pool size (too small → queuing, too large → overhead)

---

## 📈 Scalability & Limitations

### Current Limitations

**1. Single Process**
- All tasks run in one process
- Cannot scale horizontally without external coordination

**2. In-Memory Storage**
- Tasks stored in memory only
- Data lost on restart
- Limited by available RAM

**3. No Distributed Coordination**
- Cannot coordinate multiple instances
- No load balancing between instances

**4. Fixed Thread Pool**
- Thread count fixed at startup
- Cannot dynamically adjust

**5. No Task Persistence**
- Tasks not persisted to disk
- Cannot resume after crash

### Scalability Improvements (Future)

**Horizontal Scaling**:
- Distributed task queue (Redis, RabbitMQ)
- Multiple TaskWeave instances
- Load balancer in front of API

**Persistence**:
- Database backend (PostgreSQL, MongoDB)
- Task persistence and recovery
- Historical data

**Dynamic Scaling**:
- Auto-scaling thread pool
- Adaptive scheduling
- Resource monitoring

---

## 🛠️ Technology Stack

### Core Technologies

| Component | Technology | Version | Purpose |
|-----------|-----------|---------|---------|
| Language | C++ | C++17 | Core implementation |
| Threading | POSIX Threads / Windows Threads | - | Multi-threading |
| JSON Parsing | nlohmann/json | Latest | JSON processing |
| HTTP Server | cpp-httplib | Latest | REST API |
| Database | SQLite3 | 3.x | Optional persistence |
| Build System | CMake | 3.15+ | Build configuration |
| Testing | GoogleTest | Latest | Unit tests |

### Third-Party Libraries

**nlohmann/json**:
- Header-only JSON library
- Modern C++ API
- Robust error handling

**cpp-httplib**:
- Header-only HTTP server
- Simple API
- CORS support

**GoogleTest**:
- Industry-standard testing framework
- Mocking support (GMock)
- Integration with CMake

---

## ⚖️ Trade-offs & Design Decisions

### 1. In-Memory vs. Persistent Storage

**Decision**: In-memory storage (TaskRegistry)

**Trade-offs**:
- ✅ **Pros**: Fast, simple, no I/O overhead
- ❌ **Cons**: Data lost on restart, limited by RAM

**Future**: Add optional database persistence

---

### 2. Fixed vs. Dynamic Thread Pool

**Decision**: Fixed-size thread pool

**Trade-offs**:
- ✅ **Pros**: Predictable resource usage, simple implementation
- ❌ **Cons**: Cannot adapt to workload, manual tuning required

**Future**: Add dynamic thread pool with auto-scaling

---

### 3. Synchronous vs. Asynchronous API

**Decision**: Synchronous REST API

**Trade-offs**:
- ✅ **Pros**: Simple, easy to understand, standard REST
- ❌ **Cons**: Blocking operations, no real-time updates

**Future**: Add WebSocket support for real-time updates

---

### 4. Immediate Retries vs. Exponential Backoff

**Decision**: Immediate retries (no backoff)

**Trade-offs**:
- ✅ **Pros**: Simple implementation, fast recovery
- ❌ **Cons**: May overwhelm system during outages

**Future**: Add configurable retry strategies (exponential backoff, jitter)

---

### 5. Singleton vs. Dependency Injection

**Decision**: Singleton pattern for Config, Registry, Metrics

**Trade-offs**:
- ✅ **Pros**: Simple, global access, no parameter passing
- ❌ **Cons**: Harder to test, tight coupling, global state

**Future**: Consider dependency injection for testability

---

## 🚀 Future Improvements

### Short-Term

1. **Database Persistence**
   - SQLite/PostgreSQL backend
   - Task persistence and recovery
   - Historical data storage

2. **Enhanced Retry Strategies**
   - Exponential backoff
   - Configurable retry policies
   - Retry delay configuration

3. **Task Dependencies**
   - Task dependency graph
   - Sequential execution of dependent tasks
   - Dependency resolution

4. **WebSocket Support**
   - Real-time task updates
   - Live metrics streaming
   - Event notifications

### Medium-Term

5. **Dynamic Thread Pool**
   - Auto-scaling based on workload
   - Configurable min/max threads
   - Thread pool metrics

6. **Distributed Task Queue**
   - Redis/RabbitMQ integration
   - Multi-instance coordination
   - Load balancing

7. **Task Scheduling (Cron-like)**
   - Scheduled task execution
   - Recurring tasks
   - Time-based triggers

8. **Authentication & Authorization**
   - API key authentication
   - JWT tokens
   - Role-based access control

### Long-Term

9. **Web UI Dashboard**
   - Real-time task visualization
   - Metrics charts
   - Task management interface

10. **Plugin System**
    - Custom task types
    - Custom schedulers
    - Extensible architecture

11. **Monitoring & Observability**
    - Prometheus metrics export
    - Distributed tracing
    - Performance profiling

12. **Container Orchestration**
    - Kubernetes support
    - Helm charts
    - Auto-scaling policies

---

## 📚 References

- **C++17 Standard**: [cppreference.com](https://en.cppreference.com/)
- **Design Patterns**: Gang of Four (GoF) patterns
- **Concurrency**: C++ Concurrency in Action (Anthony Williams)
- **Systems Design**: Designing Data-Intensive Applications (Martin Kleppmann)

---

## 🏁 Conclusion

TaskWeave is designed as a **production-ready task execution engine** with:

- ✅ Clear separation of concerns
- ✅ Extensible architecture (Strategy pattern)
- ✅ Thread-safe operations
- ✅ Robust error handling
- ✅ Configuration flexibility
- ✅ REST API integration

The architecture balances **simplicity** and **functionality**, making it suitable for:
- Learning systems programming
- Production deployments (with persistence)
- Integration into larger systems
- Extending with new features

**Last Updated**: 2024

*For API documentation, see [API.md](API.md)*
*For usage instructions, see [README.md](README.md)*
