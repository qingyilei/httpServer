//
// Created by qing on 2025/3/5.
//

#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H
#include <sqlite3.h>
#include <queue>
#include <condition_variable>
#include <mutex>
class ConnectionPool {
public:
    explicit ConnectionPool(const std::string& db_path, size_t pool_size = 10);

    ~ConnectionPool();

    sqlite3* acquire();
    void release(sqlite3* conn);

private:
    std::string db_path_;
    std::queue<sqlite3*> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif //CONNECTION_POOL_H
