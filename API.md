# 📡 TaskWeave REST API Documentation

Complete reference documentation for the TaskWeave REST API.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Base URL](#base-url)
3. [Authentication](#authentication)
4. [Content Types](#content-types)
5. [Error Handling](#error-handling)
6. [Endpoints](#endpoints)
   - [Health Check](#health-check)
   - [Metrics](#metrics)
   - [List All Tasks](#list-all-tasks)
   - [Get Task by ID](#get-task-by-id)
   - [Submit Tasks](#submit-tasks)
   - [Web Dashboard](#web-dashboard)
7. [Data Models](#data-models)
8. [Examples](#examples)
9. [Rate Limiting](#rate-limiting)
10. [CORS Support](#cors-support)

---

## 🎯 Overview

The TaskWeave REST API provides HTTP endpoints for:

- **Task Management**: Submit, query, and monitor tasks
- **System Monitoring**: Health checks and performance metrics
- **Web Interface**: HTML dashboard for visualization

All endpoints return JSON responses (except the web dashboard which returns HTML).

---

## 🌐 Base URL

The API server runs on a configurable port (default: 8080).

```
http://localhost:8080
```

For production deployments, replace `localhost` with the appropriate hostname or IP address.

---

## 🔐 Authentication

**Current Status**: Authentication is not implemented. All endpoints are publicly accessible.

**Future**: API key authentication and JWT tokens will be added in future releases.

---

## 📦 Content Types

### Request Content Type

All POST requests should use:
```
Content-Type: application/json
```

### Response Content Type

All JSON responses use:
```
Content-Type: application/json
```

The web dashboard endpoint returns:
```
Content-Type: text/html
```

---

## ❌ Error Handling

### HTTP Status Codes

| Code | Description |
|------|-------------|
| `200` | Success |
| `400` | Bad Request (invalid JSON, malformed request) |
| `404` | Not Found (resource doesn't exist) |
| `409` | Conflict (e.g., task ID already exists) |
| `413` | Payload Too Large (request body exceeds limit) |
| `500` | Internal Server Error |

### Error Response Format

All error responses follow this format:

```json
{
  "error": "Error message description"
}
```

### Example Error Responses

**400 Bad Request:**
```json
{
  "error": "Invalid JSON format"
}
```

**404 Not Found:**
```json
{
  "error": "Task not found"
}
```

**409 Conflict:**
```json
{
  "error": "Task ID already exists"
}
```

**413 Payload Too Large:**
```json
{
  "error": "Request entity too large"
}
```

---

## 🔌 Endpoints

### Health Check

Check if the API server is running and healthy.

**Endpoint:** `GET /health`

**Description:** Returns the health status of the TaskWeave engine.

**Request:**
```http
GET /health HTTP/1.1
Host: localhost:8080
```

**Response:** `200 OK`

```json
{
  "status": "healthy",
  "engine": "running",
  "timestamp": 1704067200
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `status` | string | Always `"healthy"` if server is running |
| `engine` | string | Engine state: `"running"` or `"shutting_down"` |
| `timestamp` | integer | Unix timestamp of the response |

**Example:**
```bash
curl http://localhost:8080/health
```

---

### Metrics

Get system metrics and task statistics.

**Endpoint:** `GET /metrics` or `GET /api/metrics`

**Description:** Returns aggregated metrics about tasks and system performance.

**Request:**
```http
GET /metrics HTTP/1.1
Host: localhost:8080
```

**Response:** `200 OK`

```json
{
  "total_tasks": 150,
  "pending": 5,
  "running": 3,
  "completed": 140,
  "failed": 2,
  "uptime_seconds": 3600,
  "thread_pool_size": 4
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `total_tasks` | integer | Total number of tasks registered |
| `pending` | integer | Number of tasks in READY state |
| `running` | integer | Number of tasks currently executing |
| `completed` | integer | Number of successfully completed tasks |
| `failed` | integer | Number of failed tasks (after all retries) |
| `uptime_seconds` | integer | Server uptime in seconds (Unix timestamp) |
| `thread_pool_size` | integer | Number of worker threads in the thread pool |

**Example:**
```bash
curl http://localhost:8080/metrics
```

---

### List All Tasks

Retrieve all registered tasks with their current status.

**Endpoint:** `GET /tasks`

**Description:** Returns a list of all tasks with their metadata.

**Request:**
```http
GET /tasks HTTP/1.1
Host: localhost:8080
```

**Response:** `200 OK`

```json
{
  "tasks": [
    {
      "id": 1,
      "name": "Task 1",
      "priority": "HIGH",
      "state": 3,
      "retry_count": 0,
      "max_retries": 2,
      "type": "print",
      "created_at": "2024-01-01 00:00:00"
    },
    {
      "id": 2,
      "name": "Task 2",
      "priority": "MEDIUM",
      "state": 2,
      "retry_count": 1,
      "max_retries": 3,
      "type": "sleep",
      "created_at": "2024-01-01 00:00:00"
    }
  ]
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `tasks` | array | List of task objects |

**Task Object Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Unique task identifier |
| `name` | string | Task name (format: "Task {id}") |
| `priority` | string | Task priority: `"HIGH"`, `"MEDIUM"`, or `"LOW"` |
| `state` | integer | Task state (see [Task States](#task-states)) |
| `retry_count` | integer | Number of retry attempts made |
| `max_retries` | integer | Maximum number of retries allowed |
| `type` | string | Task type (e.g., `"print"`, `"sleep"`) |
| `created_at` | string | Task creation timestamp (format: "YYYY-MM-DD HH:MM:SS") |

**Task States (integer values):**

| Value | State | Description |
|-------|-------|-------------|
| `0` | CREATED | Task created but not yet queued |
| `1` | READY | Task queued, waiting for execution |
| `2` | RUNNING | Task currently executing |
| `6` | RETRYING | Task failed and being retried |
| `3` | COMPLETED | Task completed successfully |
| `4` | FAILED | Task failed after all retries |

**Example:**
```bash
curl http://localhost:8080/tasks
```

---

### Get Task by ID

Retrieve a specific task by its ID.

**Endpoint:** `GET /tasks/{id}`

**Description:** Returns detailed information about a specific task.

**URL Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `id` | integer | Yes | Task ID |

**Request:**
```http
GET /tasks/1 HTTP/1.1
Host: localhost:8080
```

**Response:** `200 OK`

```json
{
  "id": 1,
  "state": 3,
  "retry_count": 0,
  "max_retries": 2
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | integer | Task identifier |
| `state` | integer | Current task state (see [Task States](#task-states)) |
| `retry_count` | integer | Number of retry attempts made |
| `max_retries` | integer | Maximum number of retries allowed |

**Error Responses:**

**404 Not Found** (task doesn't exist):
```json
{
  "error": "Task not found"
}
```

**400 Bad Request** (invalid task ID):
```json
{
  "error": "Invalid task ID"
}
```

**Example:**
```bash
curl http://localhost:8080/tasks/1
```

---

### Submit Tasks

Submit one or more tasks for execution.

**Endpoint:** `POST /tasks`

**Description:** Creates and queues new tasks for execution.

**Request:**

**Headers:**
```http
Content-Type: application/json
```

**Body:**
```json
{
  "tasks": [
    {
      "id": 100,
      "name": "My Task",
      "priority": "HIGH",
      "max_retries": 2,
      "type": "print",
      "params": {
        "message": "Hello from API"
      }
    }
  ]
}
```

**Request Body Fields:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `tasks` | array | Yes | Array of task definitions |

**Task Definition Fields:**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `id` | integer | Yes | Unique task identifier (must not exist) |
| `name` | string | Yes | Task name/description |
| `priority` | string | Yes | Task priority: `"HIGH"`, `"MEDIUM"`, or `"LOW"` |
| `max_retries` | integer | Yes | Maximum number of retry attempts (≥ 0) |
| `type` | string | Yes | Task type (e.g., `"print"`, `"sleep"`) |
| `params` | object | Yes | Task-specific parameters (key-value pairs, all string values) |

**Response:** `200 OK`

```json
{
  "status": "submitted",
  "task_id": 100
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `status` | string | Status message: `"submitted"` |
| `task_id` | integer | ID of the submitted task |

**Error Responses:**

**400 Bad Request** (invalid JSON format):
```json
{
  "error": "Invalid JSON format"
}
```

**400 Bad Request** (invalid task format):
```json
{
  "error": "Invalid task format"
}
```

**409 Conflict** (task ID already exists):
```json
{
  "error": "Task ID already exists"
}
```

**413 Payload Too Large** (request body exceeds `max_request_size`):
```json
{
  "error": "Request entity too large"
}
```

**500 Internal Server Error** (unexpected error):
```json
{
  "error": "Failed to process request"
}
```

**Example:**
```bash
curl -X POST http://localhost:8080/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "tasks": [{
      "id": 100,
      "name": "API Task",
      "priority": "HIGH",
      "max_retries": 2,
      "type": "print",
      "params": {
        "message": "Hello from API"
      }
    }]
  }'
```

**Multiple Tasks:**
```bash
curl -X POST http://localhost:8080/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "tasks": [
      {
        "id": 101,
        "name": "Task 1",
        "priority": "HIGH",
        "max_retries": 2,
        "type": "print",
        "params": {"message": "Task 1 message"}
      },
      {
        "id": 102,
        "name": "Task 2",
        "priority": "MEDIUM",
        "max_retries": 1,
        "type": "sleep",
        "params": {"duration_ms": "500"}
      }
    ]
  }'
```

---

### Web Dashboard

Access the web-based dashboard for TaskWeave.

**Endpoint:** `GET /`, `GET /dashboard`, or `GET /dashboard.html`

**Description:** Returns an HTML dashboard for visualizing tasks and system metrics.

**Request:**
```http
GET /dashboard HTTP/1.1
Host: localhost:8080
```

**Response:** `200 OK`

```html
<!DOCTYPE html>
<html>
  <!-- Dashboard HTML content -->
</html>
```

**Response:** `404 Not Found` (if dashboard file not found)

**Example:**
Open in browser: `http://localhost:8080/dashboard`

Or using curl:
```bash
curl http://localhost:8080/dashboard
```

---

## 📊 Data Models

### Task Definition Schema

```json
{
  "id": "integer (required)",
  "name": "string (required)",
  "priority": "string (required, one of: HIGH, MEDIUM, LOW)",
  "max_retries": "integer (required, ≥ 0)",
  "type": "string (required)",
  "params": {
    "key": "string value"
  }
}
```

### Task Response Schema

```json
{
  "id": "integer",
  "name": "string",
  "priority": "string (HIGH|MEDIUM|LOW)",
  "state": "integer (0-6)",
  "retry_count": "integer",
  "max_retries": "integer",
  "type": "string",
  "created_at": "string (timestamp)"
}
```

### Metrics Response Schema

```json
{
  "total_tasks": "integer",
  "pending": "integer",
  "running": "integer",
  "completed": "integer",
  "failed": "integer",
  "uptime_seconds": "integer",
  "thread_pool_size": "integer"
}
```

---

## 💡 Examples

### Complete Workflow

**1. Check server health:**
```bash
curl http://localhost:8080/health
```

**2. Submit a task:**
```bash
curl -X POST http://localhost:8080/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "tasks": [{
      "id": 1,
      "name": "Process Data",
      "priority": "HIGH",
      "max_retries": 2,
      "type": "print",
      "params": {"message": "Processing..."}
    }]
  }'
```

**3. Check task status:**
```bash
curl http://localhost:8080/tasks/1
```

**4. List all tasks:**
```bash
curl http://localhost:8080/tasks
```

**5. Check metrics:**
```bash
curl http://localhost:8080/metrics
```

### Using Python

```python
import requests

BASE_URL = "http://localhost:8080"

# Health check
response = requests.get(f"{BASE_URL}/health")
print(response.json())

# Submit task
task_data = {
    "tasks": [{
        "id": 100,
        "name": "Python Task",
        "priority": "HIGH",
        "max_retries": 2,
        "type": "print",
        "params": {"message": "Hello from Python"}
    }]
}
response = requests.post(f"{BASE_URL}/tasks", json=task_data)
print(response.json())

# Get task
response = requests.get(f"{BASE_URL}/tasks/100")
print(response.json())

# Get metrics
response = requests.get(f"{BASE_URL}/metrics")
print(response.json())
```

### Using JavaScript (Fetch API)

```javascript
const BASE_URL = "http://localhost:8080";

// Health check
fetch(`${BASE_URL}/health`)
  .then(res => res.json())
  .then(data => console.log(data));

// Submit task
fetch(`${BASE_URL}/tasks`, {
  method: "POST",
  headers: {
    "Content-Type": "application/json"
  },
  body: JSON.stringify({
    tasks: [{
      id: 200,
      name: "JS Task",
      priority: "HIGH",
      max_retries: 2,
      type: "print",
      params: { message: "Hello from JavaScript" }
    }]
  })
})
  .then(res => res.json())
  .then(data => console.log(data));

// Get all tasks
fetch(`${BASE_URL}/tasks`)
  .then(res => res.json())
  .then(data => console.log(data));
```

---

## ⚡ Rate Limiting

**Current Status**: Rate limiting is not implemented. All endpoints are accessible without limits.

**Future**: Rate limiting will be added in future releases to prevent abuse.

---

## 🌍 CORS Support

TaskWeave supports Cross-Origin Resource Sharing (CORS) for web browser clients.

### Configuration

CORS is configured via the `cors_origin` setting in `config.ini`:

```ini
cors_origin=*
```

- `*` allows all origins (default)
- Specific origin: `cors_origin=http://localhost:3000`

### CORS Headers

All API responses include:

```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type
```

### Preflight Requests

OPTIONS requests are automatically handled for CORS preflight checks.

---

## 🔄 API Versioning

**Current Status**: API versioning is not implemented. All endpoints use the base path (e.g., `/tasks`).

**Future**: API versioning (e.g., `/v1/tasks`) will be added for backward compatibility.

---

## 📝 Notes

1. **Task IDs**: Must be unique. Attempting to submit a task with an existing ID will return `409 Conflict`.

2. **Task Types**: Currently supported types include `"print"` and `"sleep"`. Custom task types can be added by extending the task loader.

3. **Priority Values**: Must be exactly `"HIGH"`, `"MEDIUM"`, or `"LOW"` (case-sensitive).

4. **Params**: All parameter values in the `params` object must be strings, even numeric values (e.g., `"duration_ms": "200"`).

5. **State Values**: Task states are represented as integers. Refer to the [Task States](#task-states) table for mapping.

6. **Request Size Limit**: Default maximum request body size is 1MB. Configure via `max_request_size` in `config.ini`.

---

## 🚀 Future Enhancements

Planned API improvements:

- [ ] Authentication (API keys, JWT)
- [ ] Rate limiting
- [ ] API versioning (`/v1/`, `/v2/`)
- [ ] Task cancellation endpoint (`DELETE /tasks/{id}`)
- [ ] Task filtering and pagination for `GET /tasks`
- [ ] WebSocket support for real-time updates
- [ ] Batch operations
- [ ] Task dependency management
- [ ] Scheduled tasks (cron-like scheduling)

---

**Last Updated**: 2024

*For architecture details, see [ARCHITECTURE.md](ARCHITECTURE.md)*
*For setup instructions, see [README.md](README.md)*
