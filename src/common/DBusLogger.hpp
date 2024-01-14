#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <atomic>
#include <cstring>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOGD(...) opendbus::DBusLogger::GetInstance()->log(opendbus::DBUS_LOG_LEVEL::DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOGI(...) opendbus::DBusLogger::GetInstance()->log(opendbus::DBUS_LOG_LEVEL::INFO, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOGW(...) opendbus::DBusLogger::GetInstance()->log(opendbus::DBUS_LOG_LEVEL::WARN, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOGE(...) opendbus::DBusLogger::GetInstance()->log(opendbus::DBUS_LOG_LEVEL::ERROR, __FILENAME__, __LINE__, __VA_ARGS__)

namespace opendbus {

enum class DBUS_LOG_LEVEL: uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
};

class DBusLogger {
private:
    // std::atomic<uint8_t> log_level_;
    uint8_t log_level_;

public:
    static DBusLogger* GetInstance() {
        static DBusLogger impl;
        return &impl;
    }

    DBusLogger() {
        // log_level_.store(static_cast<uint8_t>(DBUS_LOG_LEVEL::DEBUG));
        log_level_ = static_cast<uint8_t>(DBUS_LOG_LEVEL::ERROR);
    }

    void setLogLevel(DBUS_LOG_LEVEL level) {
        // log_level_.store(static_cast<uint8_t>(level));
        log_level_ = static_cast<uint8_t>(level);
    }

    template<typename... Args>
    void log(DBUS_LOG_LEVEL level, const std::string &file, int line, Args... args) {
        
        // if (log_level_.load() > static_cast<uint8_t>(level)) return;
        if (log_level_ > static_cast<uint8_t>(level)) return;

        std::ostringstream messageStream;
        buildMessage(messageStream, args...);

        std::ostringstream logStream;
        logStream << "[" << getCurrentTime() << "] ";
        
        switch (level) {
            case DBUS_LOG_LEVEL::DEBUG:
                logStream << "[DEBUG] ";
                break;
            case DBUS_LOG_LEVEL::INFO:
                logStream << "[INFO] ";
                break;
            case DBUS_LOG_LEVEL::WARN:
                logStream << "[WARN] ";
                break;
            case DBUS_LOG_LEVEL::ERROR:
                logStream << "[ERROR] ";
                break;
        }

        logStream << "[" << file << ":" << line << "] " << messageStream.str();
        // logStream << messageStream.str();
        std::cout << logStream.str() << std::endl;
    }

private:
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto now_as_timet = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::tm now_tm = *std::localtime(&now_as_timet);
        std::ostringstream stream;
        stream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        stream << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

        return stream.str();
    }

    template<typename T, typename... Args>
    void buildMessage(std::ostringstream &stream, T& first, Args... args) {
        stream << first;
        buildMessage(stream, args...);
    }

    template<typename T, typename... Args>
    void buildMessage(std::ostringstream &stream, T& first) {
        stream << first;
    }

};

} /* namespace opendbus */
