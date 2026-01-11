#pragma once
#include <string>

class Config {
public:
    static Config& instance();

    void loadFromFile(const std::string& path);
    void loadFromArgs(int argc, char* argv[]);
    void loadFromEnvironment();

    int getThreads() const;
    std::string getScheduler() const;
    int getMaxRetries() const;
    int getApiPort() const;
    std::string getMode() const;  // "demo" or "api"
    int getMaxRequestSize() const;
    int getMaxConnections() const;
    std::string getCorsOrigin() const;
    bool isValidationEnabled() const;

    // Validation
    bool validate() const;

private:
    Config() = default;
    void validateAndSetThreads(int value);
    void validateAndSetPort(int value);
    void validateAndSetMaxRetries(int value);
    void validateAndSetScheduler(const std::string& value);
    void validateAndSetMode(const std::string& value);
    std::string getEnvVar(const std::string& name, const std::string& defaultValue = "") const;

    int threads = 2;
    std::string scheduler = "roundrobin";
    int maxRetries = 0;
    int apiPort = 8080;
    std::string mode = "demo";  // "demo" or "api"
    int maxRequestSize = 1024 * 1024;  // 1MB default
    int maxConnections = 100;
    std::string corsOrigin = "*";
    bool validationEnabled = true;
};
