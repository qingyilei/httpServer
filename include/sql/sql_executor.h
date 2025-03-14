//
// Created by qing on 2025/3/5.
//

#ifndef SQL_EXECUTOR_H
#define SQL_EXECUTOR_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "enums/operator_type.h"
#include "database/database.h"

template<class Model>
class SqlExecutor {
public:
    SqlExecutor(OperatorType operator_type, int page, int page_size,
                std::string &count_sql, std::string &operator_sql)
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

    void save_execute(Model &obj) {
        auto &db = Database::instance();
        const std::string &id = ModelTraits<Model>::instance().primary_key();
        int count = obj.field_count_primary();
        std::cout<<"operator sql:"<<operator_sql_<<std::endl;
        int saveId = (count == 0 || obj.template get<int>("id") == 0) ?
                     db.insert(this->operator_sql_, obj)
                     : db.update(this->operator_sql_, obj);
        obj.set("id", saveId);
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
