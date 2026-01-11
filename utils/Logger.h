#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>

class Logger {
public:
    static void initialize(const std::string& logFile = "taskweave.log", bool enableFileLogging = true) {
        instance().logFilePath = logFile;
        instance().fileLoggingEnabled = enableFileLogging;
        if (enableFileLogging) {
            instance().logFile.open(logFile, std::ios::app);
        }
    }
    
    static void info(const std::string& msg) {
        instance().log("INFO", msg);
    }

    static void warn(const std::string& msg) {
        instance().log("WARN", msg);
    }

    static void error(const std::string& msg) {
        instance().log("ERROR", msg);
    }
    
    static void shutdown() {
        if (instance().logFile.is_open()) {
            instance().logFile.close();
        }
    }

private:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }
    
    void log(const std::string& level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        
        std::string timestamp = getTimestamp();
        std::string logMsg = "[" + timestamp + "] [" + level + "] " + msg;
        
        // Console output
        if (level == "ERROR") {
            std::cerr << logMsg << std::endl;
        } else {
            std::cout << logMsg << std::endl;
        }
        
        // File output
        if (fileLoggingEnabled && logFile.is_open()) {
            logFile << logMsg << std::endl;
            logFile.flush();
        }
    }
    
    std::string getTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    
    std::mutex mtx;
    std::ofstream logFile;
    std::string logFilePath;
    bool fileLoggingEnabled = false;
};
