//
// Created by qing on 2025/3/5.
//

#ifndef SQL_EXECUTOR_H
#define SQL_EXECUTOR_H

#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include <iostream>
#include "enums/operator_type.h"
#include "database/database.h"
#include "log/logger.h"

template<class Model>
class SqlExecutor {
public:
    SqlExecutor(OperatorType operator_type, int page, int page_size,
                std::string &&count_sql, std::string &&operator_sql)
            : operator_type_(operator_type), page_(page), page_size_(page_size),
              count_sql_(count_sql), operator_sql_(operator_sql) {
        std::cout << "SqlExecutor init" << std::endl;
    }

    ~SqlExecutor() {
        std::cout << "SqlExecutor destroy" << std::endl;
    }

    QueryResult<Model> query_execute() {
        int total = Database::instance().query_count(count_sql_);
        auto result = Database::instance().query_data<Model>(operator_sql_);
        return result.with_metadata({
                                            page_,
                                            page_size_,
                                            total,
                                            (total + page_size_ - 1) / page_size_
                                    });
    }

    int save_execute(Model &obj) {
        auto &db = Database::instance();
        int count = obj.field_count_primary();
        std::cout << "operator sql:" << operator_sql_ << std::endl;
        int saveId = (count == 0 || obj.template get<int>("id") == 0) ?
                     db.insert(this->operator_sql_, obj)
                                                                      : db.update(this->operator_sql_, obj);
        obj.set("id", saveId);
        return saveId;
    }

    int batch_execute(std::vector<Model> &objs) {
        if (objs.empty()) {
            return 0;
        }
        std::cout << "operator sql:" << operator_sql_ << std::endl;

        auto future =  Database::instance().batch_insert(this->operator_sql_, objs, 1000);
        try {
            future.get(); // 捕获可能的异常
        } catch (const std::exception& e) {
            Logger::getInstance().error(std::format("execute batch  failed:{}",e.what()));
            throw std::runtime_error("execute batch  failed");
        }
        return static_cast<int>(objs.size());
    }

    int remove_execute(Model &obj) {
        return Database::instance().remove(this->operator_sql_, obj);
    }


private:
    /**
     * 1. select|update|delete
     * 2. select or update fields
     * 3. form table
     * 4. where condition
     */
    int page_;
    int page_size_;
    OperatorType operator_type_;
    std::string count_sql_;
    std::string operator_sql_;
};

#endif
