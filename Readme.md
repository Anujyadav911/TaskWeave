# 🚀 TaskWeave - Task Execution Engine

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A **production-grade task execution engine** built in C++ that handles concurrent task scheduling, execution, and management with REST API support.

---

## 📖 Quick Overview

**TaskWeave** is a smart system that:
- ✅ Executes multiple tasks concurrently using a thread pool
- ✅ Schedules tasks using priority-based or round-robin algorithms
- ✅ Provides a REST API for task submission and querying
- ✅ Handles task failures with automatic retry mechanisms
- ✅ Tracks task states and performance metrics
- ✅ Supports configuration-driven behavior

**Think of it as:** A restaurant kitchen manager that coordinates multiple chefs (threads) to cook orders (tasks) efficiently, with a system to track order status.

---

## 🎯 What Can You Do With This?

### Real-World Use Cases:
- **Background Job Processing**: Process images, send emails, generate reports
- **Data Pipeline**: Transform, validate, and process data batches
- **Microservices**: Task queue for distributed systems
- **Automation**: Schedule and execute automated tasks
- **Learning**: Understand multi-threading, scheduling, and system design

---

## 🚀 Quick Start

### Prerequisites
- C++ compiler with C++17 support (g++ or clang++)
- Windows: MinGW-w64 or Visual Studio
- Linux: `sudo apt-get install build-essential`
- macOS: `xcode-select --install`

### Build & Run

**Windows (PowerShell):**
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
  -o taskweave.exe -pthread -lws2_32

.\taskweave.exe
```

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
  -o taskweave -pthread

./taskweave
```

---

## 📚 Documentation

### 🎓 **NEW: Complete Beginner Guide**
**👉 [COMPLETE_GUIDE.md](COMPLETE_GUIDE.md)** - Everything from zero to advanced:
- What the project does and why it's useful
- Technologies explained in simple terms
- Step-by-step setup instructions
- How everything works (architecture explained)
- Usage examples
- Advanced topics
- Learning path
- Troubleshooting guide

### Other Guides:
- **[QUICK_START.md](QUICK_START.md)** - Quick reference for running and using TaskWeave
- **[PRODUCTION_GUIDE.md](PRODUCTION_GUIDE.md)** - Production deployment and advanced usage

---

## 🏗️ Project Structure

```
TaskWeave/
├── src/                    # Source code
│   ├── core/              # Core functionality (Task, TaskRegistry, TaskLoader)
│   ├── executor/          # Thread pool implementation
│   ├── scheduler/         # Scheduling algorithms (Priority, Round-Robin)
│   └── main.cpp           # Entry point
├── api/                   # REST API server
├── utils/                 # Utilities (Config, Logger, Metrics)
├── config.ini             # Configuration file
├── tasks.json             # Example task definitions
└── COMPLETE_GUIDE.md      # Comprehensive beginner guide
```

---

## 💻 Basic Usage

### 1. Demo Mode (Default)
Runs demonstration phases showing different features:
```bash
./taskweave
```

### 2. API Mode
Start the REST API server:
```bash
./taskweave --mode=api --api-port=8080
```

### 3. Custom Configuration
```bash
./taskweave --threads=8 --scheduler=roundrobin --max-retries=3
```

### 4. Submit Tasks via JSON
Create `tasks.json`:
```json
{
  "tasks": [
    {
      "id": 1,
      "name": "Process Data",
      "priority": "HIGH",
      "max_retries": 2,
      "type": "print",
      "params": {
        "message": "Hello World"
      }
    }
  ]
}
```

### 5. Use REST API
```bash
# Health check
curl http://localhost:8080/health

# Submit task
curl -X POST http://localhost:8080/tasks \
  -H "Content-Type: application/json" \
  -d '{"tasks":[{"id":100,"name":"API Task","priority":"HIGH","max_retries":2,"type":"print","params":{"message":"Hello"}}]}'

# Get all tasks
curl http://localhost:8080/tasks

# Get task by ID
curl http://localhost:8080/tasks/100
```

---

## ⚙️ Configuration

Edit `config.ini`:
```ini
threads=4              # Number of worker threads
scheduler=priority     # "priority" or "roundrobin"
max_retries=2          # Max retry attempts for failed tasks
api_port=8080          # REST API server port
mode=demo              # "demo" or "api"
```

---

## 🛠️ Technologies

- **C++17**: Core language
- **POSIX Threads**: Multi-threading support
- **JSON**: Task definitions
- **HTTP/REST**: API communication
- **Socket Programming**: HTTP server implementation

---

## 🎓 Learning Path

### Beginner (Start Here!)
1. Read [COMPLETE_GUIDE.md](COMPLETE_GUIDE.md) - Full explanation for beginners
2. Build and run the project
3. Modify `config.ini` and observe changes
4. Create your own `tasks.json`

### Intermediate
1. Read source code (start with `main.cpp`)
2. Understand thread pool and scheduler implementations
3. Test REST API endpoints
4. Add a new task type

### Advanced
1. Add database persistence (SQLite)
2. Write unit tests (GoogleTest)
3. Replace JSON parser with `nlohmann/json`
4. Add CMake build system
5. Create web UI dashboard

---

## 🎯 Key Features

- ✅ **Multi-threading**: Concurrent task execution with thread pool
- ✅ **Scheduling**: Priority-based and round-robin algorithms
- ✅ **REST API**: Submit and query tasks via HTTP
- ✅ **State Management**: Complete task lifecycle tracking
- ✅ **Retry Mechanism**: Automatic retry for failed tasks
- ✅ **Configuration**: No hardcoding, fully configurable
- ✅ **Metrics**: Performance tracking and statistics
- ✅ **Graceful Shutdown**: Clean shutdown handling

---

## 📊 Architecture

```
API Server → Task Registry → Scheduler → Thread Pool → Metrics
                ↓              ↓             ↓
            JSON Tasks    Priority/RR    Worker Threads
```

**Flow:**
1. Tasks submitted via API or JSON file
2. Task Registry tracks all tasks
3. Scheduler decides execution order
4. Thread Pool executes tasks concurrently
5. Metrics record performance data

---

## 🐛 Troubleshooting

**Build Errors?**
- Ensure C++17 compiler is installed
- Windows: Add `-lws2_32` flag for sockets
- Linux: Ensure `-pthread` flag is included

**Port Already in Use?**
```bash
# Use different port
./taskweave --mode=api --api-port=8081
```

**API Not Responding?**
- Check if server started successfully
- Verify firewall settings
- Check port number in config

For more help, see [COMPLETE_GUIDE.md](COMPLETE_GUIDE.md#common-problems--solutions)

---

## 🚧 Future Enhancements

- [ ] Database persistence (SQLite/PostgreSQL)
- [ ] Better JSON parser (nlohmann/json)
- [ ] Better HTTP library (cpp-httplib)
- [ ] Unit tests (GoogleTest)
- [ ] CMake build system
- [ ] Web UI dashboard
- [ ] Docker containerization
- [ ] Distributed task queue

---

## 📝 License

MIT License - Feel free to use this project for learning and building!

---

## 🤝 Contributing

Contributions welcome! This is a learning project, so feel free to:
- Report bugs
- Suggest features
- Submit pull requests
- Improve documentation

---

## 📞 Support

- **New to the project?** → Start with [COMPLETE_GUIDE.md](COMPLETE_GUIDE.md)
- **Quick questions?** → Check [QUICK_START.md](QUICK_START.md)
- **Production usage?** → Read [PRODUCTION_GUIDE.md](PRODUCTION_GUIDE.md)
- **Stuck?** → Check troubleshooting section in COMPLETE_GUIDE.md

---

## 💡 What This Project Demonstrates

This project showcases:
- ✅ Systems programming in C++
- ✅ Multi-threading and concurrency
- ✅ Design patterns (Strategy, Factory)
- ✅ REST API implementation
- ✅ Configuration management
- ✅ Production-ready features (logging, metrics, graceful shutdown)

**Perfect for:**
- Learning C++ and systems programming
- Understanding task scheduling algorithms
- Building portfolio projects
- Interview preparation

---

**Happy Coding! 🚀**

*For a complete beginner-friendly explanation, see [COMPLETE_GUIDE.md](COMPLETE_GUIDE.md)*
