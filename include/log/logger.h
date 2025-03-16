//
// Created by qing on 25-3-15.
//

#ifndef HTTPSERVER_LOGGER_H
#define HTTPSERVER_LOGGER_H

#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <boost/json.hpp>

enum class LogLevel {
    DEBUG, INFO, WARNING, ERROR
};

class Logger {
public:
    static Logger &getInstance() {
        thread_local static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        current_level_ = level;
    }

    // 修改LogEntry结构体，添加线程ID字段
    struct LogEntry {
        LogLevel level;
        std::thread::id thread_id; // 新增线程ID字段
        std::string message;
    };

    void error(const std::string &msg) {
        log(LogLevel::ERROR, msg);
    }

    void log(LogLevel level, const std::string &msg) {
        if (level >= current_level_) {
            // 保持原有输出功能
            std::cout << "[Thread " << std::this_thread::get_id()
                      << "] [" << levelToString(level) << "] " << msg << std::endl;

            // 存储时添加当前线程ID
            logs_.emplace_back(
                    level,
                    std::this_thread::get_id(), // 新增线程ID参数
                    msg
            );
        }
    }

    // 获取当前线程的日志记录
    const std::vector<LogEntry> &getLogs() const { return logs_; }

    // 清空当前线程的日志
    void clearLogs() { logs_.clear(); }


    [[nodiscard]] boost::json::object to_json() const {
        boost::json::object json_obj;
        boost::json::array logs_array;
        for (const auto& entry : logs_) {
            boost::json::object entry_obj;
            entry_obj["level"] = levelToString(entry.level);
            entry_obj["thread_id"] = threadIdToString(entry.thread_id);
            entry_obj["message"] = entry.message;
            logs_array.push_back(std::move(entry_obj));
        }
        json_obj["logs"] = std::move(logs_array);
        return json_obj;
    }


private:

    std::string threadIdToString(const std::thread::id& tid) const {
        // 使用哈希值生成唯一标识（std::thread::id本身不可直接转字符串）
        std::hash<std::thread::id> hasher;
        return std::to_string(hasher(tid));
    }
    std::string levelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARNING:
                return "WARNING";
            case LogLevel::ERROR:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

    // 使用vector存储线程本地日志
    std::vector<LogEntry> logs_;
    LogLevel current_level_{LogLevel::DEBUG};
};

#endif //HTTPSERVER_LOGGER_H
