//
// Created by qing on 2025/3/5.
//
#include "database/database.h"

Database& Database::instance() {
    static Database instance;
    return instance;
}

// 事务管理
void Database::begin_transaction(sqlite3* conn) { execute(conn,"BEGIN TRANSACTION"); }
void Database::commit(sqlite3* conn) { execute(conn,"COMMIT"); }
void Database::rollback(sqlite3* conn) { execute(conn,"ROLLBACK"); }



