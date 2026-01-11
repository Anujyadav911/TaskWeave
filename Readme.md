# 🚀 TaskWeave - Production-Grade Task Execution Engine

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

**TaskWeave** is a high-performance, production-ready task execution engine built in C++17. It provides concurrent task scheduling, execution management, and a REST API for seamless integration with distributed systems.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Key Features](#key-features)
3. [System Architecture](#system-architecture)
4. [Quick Start](#quick-start)
5. [Installation](#installation)
6. [Usage](#usage)
7. [Configuration](#configuration)
8. [API Documentation](#api-documentation)
9. [Architecture Details](#architecture-details)
10. [Project Structure](#project-structure)
11. [Building from Source](#building-from-source)
12. [Docker Deployment](#docker-deployment)
13. [Testing](#testing)
14. [Troubleshooting](#troubleshooting)
15. [Contributing](#contributing)
16. [License](#license)

---

## 🎯 Overview

TaskWeave is designed to handle concurrent task execution efficiently with:

- **Multi-threaded Execution**: Thread pool architecture for concurrent task processing
- **Flexible Scheduling**: Pluggable scheduling algorithms (Priority-based, Round-Robin)
- **REST API**: HTTP endpoints for task submission, querying, and monitoring
- **State Management**: Complete task lifecycle tracking with state machine
- **Retry Mechanism**: Automatic retry with configurable limits
- **Production Features**: Graceful shutdown, metrics, logging, configuration management

### Real-World Use Cases

- **Background Job Processing**: Image processing, email sending, report generation
- **Data Pipelines**: ETL operations, data transformation, batch processing
- **Microservices**: Task queue for distributed systems
- **Automation**: Scheduled and on-demand task execution
- **Learning**: Understanding concurrent systems, scheduling algorithms, and system design

---

## ✨ Key Features

| Feature | Description |
|---------|-------------|
| **Thread Pool** | Efficient worker thread management with configurable pool size |
| **Scheduling Algorithms** | Priority-based (HIGH → MEDIUM → LOW) and Round-Robin scheduling |
| **REST API** | Complete HTTP API for task management and monitoring |
| **State Machine** | Robust task state tracking (CREATED → READY → RUNNING → COMPLETED/FAILED) |
| **Retry Logic** | Automatic retry with configurable maximum attempts |
| **Configuration Management** | File-based, environment variable, and command-line configuration |
| **Metrics & Observability** | Performance tracking, task statistics, health endpoints |
| **Graceful Shutdown** | Clean shutdown handling with signal management |
| **Thread Safety** | Thread-safe operations throughout the system |
| **Database Persistence** | SQLite integration for task persistence (optional) |

---

## 🏗️ System Architecture

### High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         TaskWeave Engine                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐         ┌──────────────┐         ┌─────────────┐│
│  │   REST API   │────────▶│   Config     │────────▶│   Logger    ││
│  │   Server     │         │   Manager    │         │             ││
│  └──────┬───────┘         └──────────────┘         └─────────────┘│
│         │                                                    │      │
│         │                                                    │      │
│         ▼                                                    ▼      │
│  ┌────────────────────────────────────────────────────────────────┐│
│  │                    Task Registry                               ││
│  │  (In-memory storage for all task instances and metadata)       ││
│  └────────────────────┬───────────────────────────────────────────┘│
│                       │                                             │
│                       ▼                                             │
│  ┌────────────────────────────────────────────────────────────────┐│
│  │                      Scheduler                                  ││
│  │  ┌──────────────────┐        ┌──────────────────┐             ││
│  │  │ Priority         │        │ Round-Robin      │             ││
│  │  │ Scheduler        │        │ Scheduler        │             ││
│  │  └──────────────────┘        └──────────────────┘             ││
│  └────────────────────┬───────────────────────────────────────────┘│
│                       │                                             │
│                       ▼                                             │
│  ┌────────────────────────────────────────────────────────────────┐│
│  │                    Thread Pool                                  ││
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐      ││
│  │  │ Worker 1 │  │ Worker 2 │  │ Worker 3 │  │ Worker N │      ││
│  │  │ Thread   │  │ Thread   │  │ Thread   │  │ Thread   │      ││
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘      ││
│  └────────────────────┬───────────────────────────────────────────┘│
│                       │                                             │
│                       ▼                                             │
│  ┌────────────────────────────────────────────────────────────────┐│
│  │                      Metrics                                    ││
│  │  (Performance tracking, task statistics, timing data)          ││
│  └────────────────────────────────────────────────────────────────┘│
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Task Lifecycle State Machine

```
                    ┌─────────┐
                    │ CREATED │  (Task object created)
                    └────┬────┘
                         │ markReady()
                         ▼
                    ┌─────────┐
                    │  READY  │  (Queued, waiting for thread)
                    └────┬────┘
                         │ execute()
                         ▼
                    ┌─────────┐
                    │ RUNNING │  (Executing on worker thread)
                    └────┬────┘
                         │
            ┌────────────┴────────────┐
            │                         │
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
                              └─────┬─────┘
                                    │ markRetry()
                                    ▼
                              ┌─────────┐
                              │  READY  │  (Back to queue)
                              └─────────┘
```

### Data Flow Diagram

```
┌──────────────┐
│   Client     │
│  (JSON/HTTP) │
└──────┬───────┘
       │
       │ POST /tasks
       ▼
┌──────────────────┐      ┌──────────────┐
│   API Server     │─────▶│ Task Loader  │
│  (httplib)       │      │ (JSON Parse) │
└──────┬───────────┘      └──────┬───────┘
       │                         │
       │                         │ Task Definition
       │                         ▼
       │                  ┌──────────────┐
       │                  │ Task Registry│
       │                  │  (Register)  │
       │                  └──────┬───────┘
       │                         │
       │                         │ Task Object
       │                         ▼
       └──────────────────▶┌──────────────┐
                          │  Scheduler    │
                          │  (Queue)      │
                          └──────┬───────┘
                                 │
                                 │ getNextTask()
                                 ▼
                          ┌──────────────┐
                          │ Thread Pool  │
                          │  (Execute)   │
                          └──────┬───────┘
                                 │
                                 │ Task State Updates
                                 ▼
                          ┌──────────────┐
                          │   Metrics    │
                          │  (Track)     │
                          └──────────────┘
```

### Thread Pool Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Thread Pool                          │
│                                                         │
│  ┌───────────────────────────────────────────────────┐ │
│  │            Task Queue (Scheduler)                 │ │
│  │  [Task1] [Task2] [Task3] ... [TaskN]            │ │
│  └───────────────────┬───────────────────────────────┘ │
│                      │                                  │
│         ┌────────────┼────────────┐                    │
│         │            │            │                    │
│         ▼            ▼            ▼                    │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐           │
│  │ Worker 1 │  │ Worker 2 │  │ Worker 3 │  ...      │
│  │ Thread   │  │ Thread   │  │ Thread   │           │
│  │          │  │          │  │          │           │
│  │ ┌──────┐ │  │ ┌──────┐ │  │ ┌──────┐ │           │
│  │ │ Task │ │  │ │ Task │ │  │ │ Task │ │           │
│  │ │ Exec │ │  │ │ Exec │ │  │ │ Exec │ │           │
│  │ └──────┘ │  │ └──────┘ │  │ └──────┘ │           │
│  └──────────┘  └──────────┘  └──────────┘           │
│                                                      │
│  Mutex & Condition Variables (Thread Synchronization)│
└──────────────────────────────────────────────────────┘
```

---

## 🚀 Quick Start

### Prerequisites

- **C++17 Compiler**: g++ (GCC 7+) or clang++ (Clang 5+)
- **CMake 3.15+** (recommended) or manual compilation
- **POSIX Threads** (pthread) or Windows threads
- **SQLite3** (optional, for database features)

### Quick Build (CMake)

```bash
# Clone or navigate to project directory
cd TaskWeave

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make  # or: cmake --build .

# Run in demo mode
./taskweave

# Run in API mode
./taskweave --mode=api --api-port=8080
```

### Quick Test

```bash
# Start API server
./taskweave --mode=api --api-port=8080

# In another terminal, test the API
curl http://localhost:8080/health
curl http://localhost:8080/tasks
```

---

## 📦 Installation

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake libsqlite3-dev

# Build and install
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

### macOS

```bash
# Install dependencies (using Homebrew)
brew install cmake sqlite

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Windows

**Using MinGW-w64:**
```powershell
# Install MinGW-w64 and CMake
# Then build:
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

**Using Visual Studio:**
```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

---

## 💻 Usage

### Demo Mode (Default)

Run demonstration phases showing different features:

```bash
./taskweave
```

This runs through:
1. Basic thread pool execution
2. Priority scheduler demonstration
3. Round-robin scheduler demonstration
4. Config-driven execution
5. Retry mechanism demonstration

### API Mode (Production)

Start the REST API server:

```bash
./taskweave --mode=api --api-port=8080
```

The server will:
- Listen on port 8080 (configurable)
- Accept HTTP requests for task management
- Serve a web dashboard at `http://localhost:8080/dashboard`
- Provide health and metrics endpoints

### Configuration Override

Override configuration via command-line arguments:

```bash
./taskweave --threads=8 --scheduler=priority --max-retries=3 --api-port=9090
```

### Load Tasks from JSON

Create a `tasks.json` file:

```json
{
  "tasks": [
    {
      "id": 1,
      "name": "Data Processing Task",
      "priority": "HIGH",
      "max_retries": 2,
      "type": "print",
      "params": {
        "message": "Processing data batch 1"
      }
    },
    {
      "id": 2,
      "name": "Background Sync",
      "priority": "MEDIUM",
      "max_retries": 1,
      "type": "sleep",
      "params": {
        "duration_ms": "200"
      }
    }
  ]
}
```

Tasks from `tasks.json` are automatically loaded when starting in API mode.

### Using the REST API

See [API.md](API.md) for complete API documentation. Quick examples:

```bash
# Health check
curl http://localhost:8080/health

# Submit a task
curl -X POST http://localhost:8080/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "tasks": [{
      "id": 100,
      "name": "My Task",
      "priority": "HIGH",
      "max_retries": 2,
      "type": "print",
      "params": {"message": "Hello from API"}
    }]
  }'

# Get all tasks
curl http://localhost:8080/tasks

# Get task by ID
curl http://localhost:8080/tasks/100

# Get metrics
curl http://localhost:8080/metrics
```

---

## ⚙️ Configuration

TaskWeave supports multiple configuration sources (loaded in order):

1. **Defaults** (hardcoded)
2. **Configuration file** (`src/config.ini` or `config.ini`)
3. **Environment variables**
4. **Command-line arguments** (highest priority)

### Configuration File (`config.ini`)

```ini
# Thread pool configuration
threads=4
scheduler=priority          # "priority" or "roundrobin"

# Task retry configuration
max_retries=2

# API server configuration
api_port=8080
max_request_size=1048576    # 1MB in bytes
max_connections=100
cors_origin=*               # CORS allowed origin

# Application mode: 'demo' or 'api'
mode=demo
```

### Environment Variables

```bash
export TASKWEAVE_THREADS=8
export TASKWEAVE_SCHEDULER=priority
export TASKWEAVE_API_PORT=8080
export TASKWEAVE_MODE=api
```

### Command-Line Arguments

```bash
./taskweave --threads=8 --scheduler=roundrobin --api-port=9090 --mode=api
```

Available arguments:
- `--threads=<N>`: Number of worker threads
- `--scheduler=<name>`: Scheduler type ("priority" or "roundrobin")
- `--max-retries=<N>`: Maximum retry attempts
- `--api-port=<port>`: API server port
- `--mode=<mode>`: Operating mode ("demo" or "api")

---

## 📚 API Documentation

TaskWeave provides a comprehensive REST API for task management. See [API.md](API.md) for complete documentation including:

- All available endpoints
- Request/response formats
- Error codes and handling
- Authentication (future)
- Rate limiting (future)

**Quick Reference:**
- `GET /health` - Health check
- `GET /metrics` - System metrics
- `GET /tasks` - List all tasks
- `GET /tasks/{id}` - Get task by ID
- `POST /tasks` - Submit new tasks
- `GET /dashboard` - Web dashboard

---

## 🏛️ Architecture Details

For detailed architecture documentation, design decisions, and implementation details, see [ARCHITECTURE.md](ARCHITECTURE.md).

**Key Architectural Components:**
- **Task Lifecycle Management**: State machine with thread-safe transitions
- **Thread Pool**: Efficient worker thread management
- **Scheduler Pattern**: Strategy pattern for pluggable scheduling algorithms
- **Configuration Management**: Singleton pattern with multi-source loading
- **REST API**: HTTP server with route handling and CORS support
- **Metrics & Observability**: Performance tracking and statistics

---

## 📁 Project Structure

```
TaskWeave/
├── src/                          # Main source code
│   ├── core/                    # Core functionality
│   │   ├── Task.h/cpp          # Task class and lifecycle
│   │   ├── TaskState.h         # Task state enumeration
│   │   ├── EngineState.h       # Engine state enumeration
│   │   └── ...
│   ├── executor/                # Execution layer
│   │   └── ThreadPool.h/cpp    # Thread pool implementation
│   ├── scheduler/               # Scheduling algorithms
│   │   ├── Scheduler.h         # Scheduler interface
│   │   ├── PriorityScheduler.h/cpp
│   │   └── RoundRobinScheduler.h/cpp
│   └── main.cpp                # Application entry point
│
├── api/                         # REST API
│   └── ApiServer.h/cpp         # HTTP server implementation
│
├── core/                        # Core utilities
│   ├── TaskDefinition.h        # JSON task schema
│   ├── TaskLoader.h/cpp        # JSON parsing and task creation
│   └── TaskRegistry.h/cpp      # Task storage and retrieval
│
├── utils/                       # Utility modules
│   ├── Config.h/cpp            # Configuration management
│   ├── Logger.h                # Logging system
│   ├── Metrics.h/cpp           # Performance metrics
│   └── Database.h/cpp          # SQLite integration
│
├── tests/                       # Unit tests
│   ├── test_task.cpp
│   ├── test_scheduler.cpp
│   └── test_task_loader.cpp
│
├── third_party/                 # Third-party libraries
│   ├── json.hpp                # nlohmann/json
│   └── httplib.h               # cpp-httplib
│
├── web/                         # Web assets
│   └── dashboard.html          # Web dashboard
│
├── build/                       # Build directory (generated)
│
├── CMakeLists.txt              # CMake build configuration
├── config.ini                   # Configuration file
├── tasks.json                   # Example task definitions
├── Dockerfile                   # Docker container definition
├── docker-compose.yml           # Docker Compose configuration
│
├── README.md                    # This file
├── API.md                       # API documentation
├── ARCHITECTURE.md              # Architecture documentation
├── PRODUCTION_GUIDE.md          # Production deployment guide
└── QUICK_START.md               # Quick start guide
```

---

## 🔨 Building from Source

### Using CMake (Recommended)

```bash
# Configure
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
make  # Linux/macOS
# or
cmake --build . --config Release  # Cross-platform

# Run tests (optional)
ctest

# Install (optional)
sudo make install
```

### Manual Compilation

**Linux/macOS:**
```bash
g++ -std=c++17 \
  src/main.cpp \
  src/core/Task.cpp \
  core/TaskLoader.cpp \
  core/TaskRegistry.cpp \
  src/executor/ThreadPool.cpp \
  src/scheduler/PriorityScheduler.cpp \
  src/scheduler/RoundRobinScheduler.cpp \
  api/ApiServer.cpp \
  utils/Config.cpp \
  utils/Metrics.cpp \
  utils/Database.cpp \
  -o taskweave -pthread -lsqlite3
```

**Windows (MinGW):**
```powershell
g++ -std=c++17 `
  src/main.cpp `
  src/core/Task.cpp `
  core/TaskLoader.cpp `
  core/TaskRegistry.cpp `
  src/executor/ThreadPool.cpp `
  src/scheduler/PriorityScheduler.cpp `
  src/scheduler/RoundRobinScheduler.cpp `
  api/ApiServer.cpp `
  utils/Config.cpp `
  utils/Metrics.cpp `
  utils/Database.cpp `
  -o taskweave.exe -pthread -lws2_32 -lsqlite3
```

---

## 🐳 Docker Deployment

### Build Docker Image

```bash
docker build -t taskweave:latest .
```

### Run Container

```bash
# Run in API mode
docker run -d -p 8080:8080 --name taskweave taskweave:latest --mode=api

# With custom configuration
docker run -d -p 8080:8080 \
  -v $(pwd)/config.ini:/app/config.ini \
  --name taskweave taskweave:latest --mode=api
```

### Docker Compose

```bash
docker-compose up -d
```

See `docker-compose.yml` for configuration.

---

## 🧪 Testing

TaskWeave includes unit tests using GoogleTest.

### Run Tests

```bash
# Build with tests enabled
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make

# Run tests
ctest

# Or run directly
./taskweave_tests
```

### Test Coverage

- Task state transitions
- Scheduler algorithms
- Task loader (JSON parsing)
- Thread pool behavior
- Configuration management

---

## 🐛 Troubleshooting

### Build Issues

**Problem**: Compiler not found
```bash
# Install build tools
# Linux: sudo apt-get install build-essential
# macOS: xcode-select --install
```

**Problem**: Missing pthread library
```bash
# Ensure -pthread flag is included in compilation
# Windows: Use -lws2_32 for socket support
```

**Problem**: SQLite not found
```bash
# Install SQLite development libraries
# Linux: sudo apt-get install libsqlite3-dev
# macOS: brew install sqlite
```

### Runtime Issues

**Problem**: Port already in use
```bash
# Use a different port
./taskweave --mode=api --api-port=8081

# Or find and kill the process using the port
# Linux: sudo lsof -i :8080
# Windows: netstat -ano | findstr :8080
```

**Problem**: API not responding
- Check if server started successfully (check logs)
- Verify firewall settings
- Ensure correct port number in configuration
- Check `taskweave.log` for errors

**Problem**: Tasks not executing
- Verify thread pool size is > 0
- Check scheduler configuration
- Review task definitions in JSON
- Check logs for error messages

### Performance Issues

- **High CPU usage**: Reduce thread pool size
- **Slow task execution**: Check task implementation, consider increasing threads
- **Memory issues**: Review task queue size, implement task limits

---

## 🤝 Contributing

Contributions are welcome! This is a learning project, so feel free to:

- Report bugs
- Suggest features
- Submit pull requests
- Improve documentation
- Add tests

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

---

## 📄 License

MIT License - See LICENSE file for details.

---

## 📞 Support & Resources

- **Complete Guide**: [COMPLETE_GUIDE.md](COMPLETE_GUIDE.md) - Beginner-friendly explanation
- **Quick Start**: [QUICK_START.md](QUICK_START.md) - Quick reference
- **Production Guide**: [PRODUCTION_GUIDE.md](PRODUCTION_GUIDE.md) - Production deployment
- **API Documentation**: [API.md](API.md) - Complete API reference
- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md) - Design and implementation details

---

## 🎓 What This Project Demonstrates

TaskWeave showcases production-grade systems programming concepts:

- ✅ **Multi-threading & Concurrency**: Thread pools, synchronization, thread safety
- ✅ **Design Patterns**: Strategy (schedulers), Singleton (config), Factory (task creation)
- ✅ **System Design**: REST APIs, state machines, configuration management
- ✅ **C++ Best Practices**: RAII, smart pointers, exception handling, modern C++17
- ✅ **Production Engineering**: Logging, metrics, graceful shutdown, error handling
- ✅ **Build Systems**: CMake, cross-platform compilation
- ✅ **Testing**: Unit tests with GoogleTest
- ✅ **DevOps**: Docker containerization, deployment strategies

**Perfect for:**
- Learning systems programming in C++
- Understanding concurrent systems and scheduling
- Building portfolio projects
- Interview preparation
- Production deployment practice

---

**Happy Coding! 🚀**

*Built with ❤️ using C++17*
