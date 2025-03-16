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
#include "log/logger.h"

#include <format>
#include <variant>
#include <iostream>
#include <future>


class Database {
public:

    static Database &instance();

    void begin_transaction(sqlite3 *conn);

    void commit(sqlite3 *conn);

    void rollback(sqlite3 *conn);


    // 插入记录（返回自增ID）
    template<typename Model>
    int insert(const std::string create_sql, const Model &obj) {
        return execute_transaction([this, &create_sql](sqlite3 *conn, const Model &insertObj) {
            auto stmt = prepare_statement(conn, create_sql);
            bind_name_params(stmt.get(), insertObj);
            execute_statement(stmt.get());
            return sqlite3_last_insert_rowid(conn);
        }, obj);
    }

    template<typename Model>
    std::future<void> batch_insert(const std::string create_sql, const std::vector<Model> &objs,size_t batch_size = 1000) {
        return std::async(std::launch::async, [this, create_sql, objs, batch_size]() {
            execute_transaction([this, &objs, &create_sql, &batch_size](sqlite3 *conn, const std::vector<Model> &list) {
                auto stmt = prepare_statement(conn, create_sql); // 预处理语句复用
                size_t total = list.size();
                for (size_t i = 0; i < total; i += batch_size) {
                    size_t batch_end = std::min(i + batch_size, total); // 计算当前批次结束位置
                    for (size_t j = i; j < batch_end; ++j) {
                        const auto &obj = list[j];
                        bind_name_params(stmt.get(), obj); // 绑定参数
                        if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
                            Logger::getInstance().error(std::format("Insert failed: {}", sqlite3_errmsg(conn)));
                            throw std::runtime_error("Insert failed: " + std::string(sqlite3_errmsg(conn)));
                        }
                        // 重置语句以便下次使用
                        sqlite3_reset(stmt.get());
                        sqlite3_clear_bindings(stmt.get());
                    }
                    // 可选：定期提交（如每批提交，但需确保事务原子性）
                }
                return objs.size();
            }, objs); // 通过execute_transaction传递参数
        });
    }


// 更新记录
    template<typename Model>
    int update(const std::string update_sql, const Model &obj) {
        return execute_transaction([this, &update_sql](sqlite3 *conn, const Model &insertObj) {
            std::cout << "update sql:" << update_sql << std::endl;
            auto stmt = prepare_statement(conn, update_sql);
            bind_name_params(stmt.get(), insertObj);
            int rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "Execution error: %s\n", sqlite3_errmsg(conn));
                throw std::runtime_error(std::format("execution update sql {} error", update_sql));
            }
            int changes = sqlite3_changes(conn);
            std::cout << "change count:" << changes << std::endl;
            return changes;
        }, obj);
    }

// 删除记录
    template<typename Model>
    int remove(const std::string &sql, const Model &obj) {
        return execute_transaction([this](sqlite3 *conn, const Model &deleteObj, decltype(sql) deleteSql) {
            auto stmt = prepare_statement(conn, deleteSql);
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
            pool_->release(conn);
            rollback(conn);
            std::cerr << "Commit failed: " << e.what() << std::endl;
            Logger::getInstance().error(std::format("execute sql failed:{}", e.what()));
            throw std::runtime_error(e.what());
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
        pool_ = std::make_unique<ConnectionPool>("users.db", 50);
    }

    std::unique_ptr<ConnectionPool> pool_;

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

    // 绑定模型参数
    template<typename Model>
    void bind_name_params(sqlite3_stmt *stmt, const Model &obj) {
        const auto &traits = ModelTraits<Model>::instance();
        int index = 1;
        for (const auto &[name, field]: traits.fields()) {
            if (name == traits.primary_key()) continue;
            int param_index = sqlite3_bind_parameter_index(stmt, std::string((":" + name)).c_str());

            std::visit([&](auto &&value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, int>) {
                    sqlite3_bind_int(stmt, param_index > 0 ? param_index : index++, value);
                } else if constexpr (std::is_same_v<T, double>) {
                    sqlite3_bind_double(stmt, param_index > 0 ? param_index : index++, value);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    sqlite3_bind_text(stmt, param_index > 0 ? param_index : index++, value.c_str(), -1,
                                      SQLITE_TRANSIENT);
                } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                    sqlite3_bind_blob(stmt, param_index > 0 ? param_index : index++, value.data(), value.size(),
                                      SQLITE_TRANSIENT);
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
