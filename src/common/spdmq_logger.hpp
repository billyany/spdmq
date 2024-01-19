#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <atomic>
// #include <cstring>

#define __FILENAME__ (strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOGD(...) opendbus::spdmq_logger::instance()->log(opendbus::LOG_LEVEL::DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOGI(...) opendbus::spdmq_logger::instance()->log(opendbus::LOG_LEVEL::INFO, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOGW(...) opendbus::spdmq_logger::instance()->log(opendbus::LOG_LEVEL::WARN, __FILENAME__, __LINE__, __VA_ARGS__)
#define LOGE(...) opendbus::spdmq_logger::instance()->log(opendbus::LOG_LEVEL::ERROR, __FILENAME__, __LINE__, __VA_ARGS__)

namespace opendbus {

enum class LOG_LEVEL: uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
};

class spdmq_logger {
private:
    std::atomic<uint8_t> log_level_;

public:
    static spdmq_logger* instance() {
        static spdmq_logger impl;
        return &impl;
    }

    spdmq_logger() {
        log_level_.store(static_cast<uint8_t>(LOG_LEVEL::DEBUG));
    }

    void set_loglevel(LOG_LEVEL level) {
        log_level_.store(static_cast<uint8_t>(level));
    }

    template<typename... Args>
    void log(LOG_LEVEL level, const std::string &file, int line, Args... args) {
        
        if (log_level_.load() > static_cast<uint8_t>(level)) return;

        std::ostringstream messageStream;
        build_message(messageStream, args...);

        std::ostringstream log_stream;
        log_stream << "[" << get_current_time() << "] ";
        
        switch (level) {
            case LOG_LEVEL::DEBUG:
                log_stream << "[DEBUG] ";
                break;
            case LOG_LEVEL::INFO:
                log_stream << "[INFO] ";
                break;
            case LOG_LEVEL::WARN:
                log_stream << "[WARN] ";
                break;
            case LOG_LEVEL::ERROR:
                log_stream << "[ERROR] ";
                break;
        }

        log_stream << "[" << file << ":" << line << "] " << messageStream.str();
        // log_stream << messageStream.str();
        std::cout << log_stream.str() << std::endl;
    }

private:
    std::string get_current_time() {
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
    void build_message(std::ostringstream &stream, T& first, Args... args) {
        stream << first;
        build_message(stream, args...);
    }

    template<typename T, typename... Args>
    void build_message(std::ostringstream &stream, T& first) {
        stream << first;
    }

};

} /* namespace opendbus */
