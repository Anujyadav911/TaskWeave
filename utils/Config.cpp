#include "Config.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstring>
#endif

Config& Config::instance() {
    static Config cfg;
    return cfg;
}

std::string Config::getEnvVar(const std::string& name, const std::string& defaultValue) const {
#ifdef _WIN32
    // Use getenv for MinGW compatibility (works on both MSVC and MinGW)
    // _dupenv_s is MSVC-only and not available in MinGW
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
#else
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
#endif
}

void Config::validateAndSetThreads(int value) {
    if (value < 1 || value > 128) {
        Logger::warn("Invalid thread count: " + std::to_string(value) + ". Using default: 2");
        threads = 2;
    } else {
        threads = value;
    }
}

void Config::validateAndSetPort(int value) {
    if (value < 1024 || value > 65535) {
        Logger::warn("Invalid port: " + std::to_string(value) + ". Using default: 8080");
        apiPort = 8080;
    } else {
        apiPort = value;
    }
}

void Config::validateAndSetMaxRetries(int value) {
    if (value < 0 || value > 100) {
        Logger::warn("Invalid max_retries: " + std::to_string(value) + ". Using default: 0");
        maxRetries = 0;
    } else {
        maxRetries = value;
    }
}

void Config::validateAndSetScheduler(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "priority" || lower == "roundrobin" || lower == "round-robin") {
        scheduler = (lower == "priority") ? "priority" : "roundrobin";
    } else {
        Logger::warn("Invalid scheduler: " + value + ". Using default: roundrobin");
        scheduler = "roundrobin";
    }
}

void Config::validateAndSetMode(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "demo" || lower == "api") {
        mode = lower;
    } else {
        Logger::warn("Invalid mode: " + value + ". Using default: demo");
        mode = "demo";
    }
}

void Config::loadFromEnvironment() {
    std::string envThreads = getEnvVar("TASKWEAVE_THREADS");
    if (!envThreads.empty()) {
        try {
            validateAndSetThreads(std::stoi(envThreads));
        } catch (...) {
            Logger::warn("Invalid TASKWEAVE_THREADS environment variable");
        }
    }

    std::string envPort = getEnvVar("TASKWEAVE_API_PORT");
    if (!envPort.empty()) {
        try {
            validateAndSetPort(std::stoi(envPort));
        } catch (...) {
            Logger::warn("Invalid TASKWEAVE_API_PORT environment variable");
        }
    }

    std::string envScheduler = getEnvVar("TASKWEAVE_SCHEDULER");
    if (!envScheduler.empty()) {
        validateAndSetScheduler(envScheduler);
    }

    std::string envMode = getEnvVar("TASKWEAVE_MODE");
    if (!envMode.empty()) {
        validateAndSetMode(envMode);
    }

    std::string envRetries = getEnvVar("TASKWEAVE_MAX_RETRIES");
    if (!envRetries.empty()) {
        try {
            validateAndSetMaxRetries(std::stoi(envRetries));
        } catch (...) {
            Logger::warn("Invalid TASKWEAVE_MAX_RETRIES environment variable");
        }
    }

    std::string envCors = getEnvVar("TASKWEAVE_CORS_ORIGIN");
    if (!envCors.empty()) {
        corsOrigin = envCors;
    }

    std::string envMaxReq = getEnvVar("TASKWEAVE_MAX_REQUEST_SIZE");
    if (!envMaxReq.empty()) {
        try {
            int size = std::stoi(envMaxReq);
            if (size > 0 && size <= 10 * 1024 * 1024) {  // Max 10MB
                maxRequestSize = size;
            }
        } catch (...) {
            Logger::warn("Invalid TASKWEAVE_MAX_REQUEST_SIZE environment variable");
        }
    }
}

void Config::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::warn("Config file not found: " + path + ". Using defaults.");
        return;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string key, value;

        if (std::getline(iss, key, '=')) {
            // Trim key
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            
            if (std::getline(iss, value)) {
                // Trim value
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                try {
                    if (key == "threads")
                        validateAndSetThreads(std::stoi(value));
                    else if (key == "scheduler")
                        validateAndSetScheduler(value);
                    else if (key == "max_retries")
                        validateAndSetMaxRetries(std::stoi(value));
                    else if (key == "api_port")
                        validateAndSetPort(std::stoi(value));
                    else if (key == "mode")
                        validateAndSetMode(value);
                    else if (key == "max_request_size") {
                        int size = std::stoi(value);
                        if (size > 0 && size <= 10 * 1024 * 1024) {
                            maxRequestSize = size;
                        }
                    } else if (key == "max_connections") {
                        int conns = std::stoi(value);
                        if (conns > 0 && conns <= 1000) {
                            maxConnections = conns;
                        }
                    } else if (key == "cors_origin") {
                        corsOrigin = value;
                    } else {
                        Logger::warn("Unknown config key: " + key + " at line " + std::to_string(lineNum));
                    }
                } catch (const std::exception& e) {
                    Logger::error("Error parsing config at line " + std::to_string(lineNum) + ": " + e.what());
                }
            }
        }
    }
}

void Config::loadFromArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        try {
            if (arg.find("--threads=") == 0)
                validateAndSetThreads(std::stoi(arg.substr(10)));
            else if (arg.find("--scheduler=") == 0)
                validateAndSetScheduler(arg.substr(12));
            else if (arg.find("--max-retries=") == 0)
                validateAndSetMaxRetries(std::stoi(arg.substr(14)));
            else if (arg.find("--api-port=") == 0)
                validateAndSetPort(std::stoi(arg.substr(11)));
            else if (arg.find("--mode=") == 0)
                validateAndSetMode(arg.substr(7));
            else if (arg.find("--max-request-size=") == 0) {
                int size = std::stoi(arg.substr(19));
                if (size > 0 && size <= 10 * 1024 * 1024) {
                    maxRequestSize = size;
                }
            } else if (arg.find("--cors-origin=") == 0) {
                corsOrigin = arg.substr(14);
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "TaskWeave Configuration Options:\n"
                          << "  --threads=N              Number of worker threads (1-128)\n"
                          << "  --scheduler=TYPE         Scheduler type (priority|roundrobin)\n"
                          << "  --max-retries=N          Maximum retry attempts (0-100)\n"
                          << "  --api-port=N             API server port (1024-65535)\n"
                          << "  --mode=MODE              Mode (demo|api)\n"
                          << "  --max-request-size=N     Max request size in bytes\n"
                          << "  --cors-origin=ORIGIN     CORS origin (default: *)\n"
                          << "\nEnvironment Variables:\n"
                          << "  TASKWEAVE_THREADS, TASKWEAVE_API_PORT, TASKWEAVE_SCHEDULER,\n"
                          << "  TASKWEAVE_MODE, TASKWEAVE_MAX_RETRIES, TASKWEAVE_CORS_ORIGIN\n";
            }
        } catch (const std::exception& e) {
            Logger::error("Error parsing argument: " + arg + " - " + e.what());
        }
    }
}

int Config::getThreads() const {
    return threads;
}

std::string Config::getScheduler() const {
    return scheduler;
}

int Config::getMaxRetries() const {
    return maxRetries;
}

int Config::getApiPort() const {
    return apiPort;
}

std::string Config::getMode() const {
    return mode;
}

int Config::getMaxRequestSize() const {
    return maxRequestSize;
}

int Config::getMaxConnections() const {
    return maxConnections;
}

std::string Config::getCorsOrigin() const {
    return corsOrigin;
}

bool Config::isValidationEnabled() const {
    return validationEnabled;
}

bool Config::validate() const {
    bool valid = true;
    
    if (threads < 1 || threads > 128) {
        Logger::error("Invalid thread count: " + std::to_string(threads));
        valid = false;
    }
    
    if (apiPort < 1024 || apiPort > 65535) {
        Logger::error("Invalid API port: " + std::to_string(apiPort));
        valid = false;
    }
    
    if (maxRetries < 0 || maxRetries > 100) {
        Logger::error("Invalid max_retries: " + std::to_string(maxRetries));
        valid = false;
    }
    
    std::string lowerSched = scheduler;
    std::transform(lowerSched.begin(), lowerSched.end(), lowerSched.begin(), ::tolower);
    if (lowerSched != "priority" && lowerSched != "roundrobin" && lowerSched != "round-robin") {
        Logger::error("Invalid scheduler: " + scheduler);
        valid = false;
    }
    
    std::string lowerMode = mode;
    std::transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(), ::tolower);
    if (lowerMode != "demo" && lowerMode != "api") {
        Logger::error("Invalid mode: " + mode);
        valid = false;
    }
    
    return valid;
}
