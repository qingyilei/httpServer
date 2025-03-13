//
// Created by qing on 2025/3/5.
//

#include "database/connection_pool.h"
#include <stdexcept>

ConnectionPool::ConnectionPool(const std::string& db_path, size_t pool_size)
    : db_path_(db_path) {
    for (size_t i = 0; i < pool_size; ++i) {
        sqlite3* conn = nullptr;
        if (sqlite3_open(db_path.c_str(), &conn) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(conn));
        }
        pool_.push(conn);
    }
}

sqlite3* ConnectionPool::acquire() {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [this]{ return !pool_.empty(); });
    auto conn = pool_.front();
    pool_.pop();
    return conn;
}
void ConnectionPool::release(sqlite3* conn) {
    std::unique_lock lock(mutex_);
    pool_.push(conn);
    cond_.notify_one();
}

ConnectionPool::~ConnectionPool() {
    while (!pool_.empty()) {
        sqlite3_close(pool_.front());
        pool_.pop();
    }
}
