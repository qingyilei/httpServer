//
// Created by qing on 2025/3/5.
//

#ifndef DATABASE_DATABASE_H
#define DATABASE_DATABASE_H
#pragma once

#include <memory>
#include "connection_pool.h"
#include "model/model_traits.h"
#include "query_result.h"


#include <format>
#include <variant>
#include <iostream>


class Database {
public:

    static Database &instance();

    void begin_transaction(sqlite3 *conn);

    void commit(sqlite3 *conn);

    void rollback(sqlite3 *conn);

// 创建表（自动生成DDL）
    template<typename Model>
    void create_table() {
        const auto &traits = ModelTraits<Model>::instance();
        std::string sql = traits.generate_create_table_ddl();
        execute_transaction([this](sqlite3 *conn, std::string &createSql) {
            execute(conn, createSql);
        }, sql);
    }

    // 插入记录（返回自增ID）
    template<typename Model>
    int insert(const Model &obj) {
        return execute_transaction([this](sqlite3 *conn, const Model &insertObj) {
            auto sql = generate_insert_sql<Model>();
            auto stmt = prepare_statement(conn, sql);
            bind_model_params(stmt.get(), insertObj);
            execute_statement(stmt.get());
            return sqlite3_last_insert_rowid(conn);
        }, obj);
    }


// 更新记录
    template<typename Model>
    int update(const Model &obj) {
        return execute_transaction([this](sqlite3 *conn, const Model &insertObj) {
            auto sql = generate_update_sql<Model>();
            std::cout << "update sql:" << sql << std::endl;
            auto stmt = prepare_statement(conn, sql);
            bind_model_params(stmt.get(), insertObj);
            bind_primary_key(stmt.get(), insertObj);
            int rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "Execution error: %s\n", sqlite3_errmsg(conn));
                throw std::runtime_error(std::format("execution update sql {} error", sql));
            }
            int changes = sqlite3_changes(conn);
            std::cout << "change count:" << changes << std::endl;
            return changes;
        }, obj);
    }


// 删除记录
    template<typename Model>
    int remove(const Model &obj) {
        const auto &traits = ModelTraits<Model>::instance();
        auto sql = std::format("DELETE FROM {} WHERE {} = ?", traits.table_name(),
                               traits.primary_key());
        return execute_transaction([this](sqlite3 *conn, const Model &deleteObj, decltype(sql) deleteSql) {
            auto stmt = prepare_statement(conn, deleteSql);
            bind_primary_key(stmt.get(), deleteObj);
            return execute_statement(stmt.get());
        }, obj, sql);

    }

    template<typename Func, typename... Args>
    auto execute_transaction(Func &&func, Args &&... args)
    -> decltype(func(std::declval<sqlite3 *>(), std::forward<Args>(args)...)) {
        auto conn = pool_->acquire();  // 从连接池获取专用连接
        try {
            begin_transaction(conn);  // 开启事务
            if constexpr (std::is_void_v<std::invoke_result_t<Func, decltype(conn), Args...>>) {
                func(conn, std::forward<Args>(args)...); // void返回类型直接执行
                commit(conn);
                pool_->release(conn);
                return; // 补充一个无意义的返回值或调整模板逻辑
            }
            auto result = func(conn, std::forward<Args>(args)...); // 非void类型保存结果
            commit(conn);
            pool_->release(conn);
            return result;
        } catch (const std::exception &e) {
            rollback(conn);
            pool_->release(conn);
            std::cerr << "Commit failed: " << e.what() << std::endl;
            throw std::runtime_error("rollback error");
        }
    }

    template<typename Model>
    QueryResult<Model> query_data(const std::string &sql) {
        sqlite3 *db = pool_->acquire();
        sqlite3_stmt *stmt = nullptr;
        std::cout << "query sql:" << sql << std::endl;
        // 准备语句
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Prepare failed: " + std::string(sqlite3_errmsg(db)));
        }

        // 执行查询
        std::vector<Model> results;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            results.push_back(parse_row<Model>(stmt));
        }
        // 清理资源
        sqlite3_finalize(stmt);
        pool_->release(db);
        return QueryResult<Model>(results);
    }

    int query_count(const std::string &sql) {
        std::cout << "count sql:" << sql << std::endl;
        sqlite3 *db = pool_->acquire();
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Prepare failed: " + std::string(sqlite3_errmsg(db)));
        }
        // 执行查询
        int result = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
        // 清理资源
        sqlite3_finalize(stmt);
        pool_->release(db);
        return result;
    }

private:
    Database() {
        pool_ = std::make_unique<ConnectionPool>("users.db", 10);
    }

    std::unique_ptr<ConnectionPool> pool_;

    // 生成INSERT SQL
    template<typename Model>
    std::string generate_insert_sql() {
        const auto &traits = ModelTraits<Model>::instance();

        std::string columns;
        std::string placeholders;
        for (const auto &[name, _]: traits.fields()) {
            if (name == traits.primary_key()) continue;

            if (!columns.empty()) columns += ", ";
            columns += name;

            if (!placeholders.empty()) placeholders += ", ";
            placeholders += "?";
        }

        return std::format("INSERT INTO {} ({}) VALUES ({})", traits.table_name(), columns, placeholders);
    }

    template<typename Model>
    Model parse_row(sqlite3_stmt *stmt) const {
        Model obj;
        const int column_count = sqlite3_column_count(stmt);
        for (int i = 0; i < column_count; ++i) {
            const char *col_name = sqlite3_column_name(stmt, i);
            const int col_type = sqlite3_column_type(stmt, i);

            switch (col_type) {
                case SQLITE_INTEGER:
                    ModelTraits<Model>::instance().set_field(col_name, obj, sqlite3_column_int(stmt, i));
                    break;
                case SQLITE_FLOAT:
                    ModelTraits<Model>::instance().set_field(col_name, obj, sqlite3_column_double(stmt, i));

                    break;
                case SQLITE_TEXT:
                    ModelTraits<Model>::instance().set_field(col_name, obj, std::string(
                            reinterpret_cast<const char *>(sqlite3_column_text(stmt, i))));
                    break;
                case SQLITE_BLOB: {
                    const auto *blob_data = static_cast<const uint8_t *>(sqlite3_column_blob(stmt, i));
                    int blob_size = sqlite3_column_bytes(stmt, i);
                    ModelTraits<Model>::instance().set_field(col_name, obj,
                                                             std::vector<uint8_t>(blob_data, blob_data + blob_size));
                    break;
                }
                case SQLITE_NULL:
                    ModelTraits<Model>::instance().set_field(col_name, obj, nullptr);
                    break;
                default:
                    throw std::runtime_error("Unsupported column type");
            }
        }
        return obj;
    }

    // 生成UPDATE SQL
    template<typename Model>
    std::string generate_update_sql() {
        const auto &traits = ModelTraits<Model>::instance();

        std::string set_clause;
        for (const auto &[name, _]: traits.fields()) {
            if (name == traits.primary_key()) continue;

            if (!set_clause.empty()) set_clause += ", ";
            set_clause += name + " = ?";
        }

        return std::format("UPDATE {} SET {} WHERE {}= ?", traits.table_name(), set_clause, traits.primary_key());
    }

    // 绑定模型参数
    template<typename Model>
    void bind_model_params(sqlite3_stmt *stmt, const Model &obj) {
        const auto &traits = ModelTraits<Model>::instance();
        int index = 1;

        for (const auto &[name, field]: traits.fields()) {
            if (name == traits.primary_key()) continue;

            std::visit([&](auto &&value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, int>) {
                    sqlite3_bind_int(stmt, index++, value);
                } else if constexpr (std::is_same_v<T, double>) {
                    sqlite3_bind_double(stmt, index++, value);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    sqlite3_bind_text(stmt, index++, value.c_str(), -1, SQLITE_TRANSIENT);
                } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                    sqlite3_bind_blob(stmt, index++, value.data(), value.size(), SQLITE_TRANSIENT);
                }
            }, obj.get_field_value(name));
        }
    }

    // 绑定主键参数
    template<typename Model>
    void bind_primary_key([[maybe_unused]] sqlite3_stmt *stmt, const Model &obj) {
        const auto &traits = ModelTraits<Model>::instance();
        auto pk_value = obj.get_field_value(traits.primary_key());
        std::visit([&](auto &&value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>) {
                sqlite3_bind_int(stmt, sqlite3_bind_parameter_count(stmt), value);
            } else {
                throw std::runtime_error("Unsupported primary key type");
            }
        }, pk_value);
    }


// 执行原生SQL
    static void execute(sqlite3 *connect, const std::string &sql) {
        char *err_msg = nullptr;
        if (sqlite3_exec(connect, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string msg(err_msg);
            sqlite3_free(err_msg);
            throw std::runtime_error("SQL Error: " + msg);
        }
    }

// 准备预处理语句
    static std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
    prepare_statement(sqlite3 *conn, const std::string &sql) {
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(conn, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(conn));
        }
        return {stmt, sqlite3_finalize};
    }

// 执行预处理语句
    static int execute_statement(sqlite3_stmt *stmt) {
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            throw std::runtime_error(sqlite3_errmsg(sqlite3_db_handle(stmt)));
        }
        return sqlite3_changes(sqlite3_db_handle(stmt));
    }
};

#endif // DATABASE_DATABASE_H
