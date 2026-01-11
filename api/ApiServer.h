#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "../src/executor/ThreadPool.h"

// Forward declarations
namespace httplib {
    class Server;
    class Response;
    class Request;
}

class ApiServer {
public:
    ApiServer(std::shared_ptr<ThreadPool> pool, int port);
    ~ApiServer();
    
    void start();
    void stop();
    
private:
    void setupRoutes();
    void setCorsHeaders(httplib::Response& res);
    
    std::shared_ptr<ThreadPool> threadPool;
    std::unique_ptr<httplib::Server> server;
    int port;
    std::atomic<bool> running;
    std::thread serverThread;
    int maxRequestSize;
    std::string corsOrigin;
};

