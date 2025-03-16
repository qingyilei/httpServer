//
// Created by qing on 2025/3/5.
//

#ifndef SQL_PAGE_H
#define SQL_PAGE_H

#include <string>
#include <vector>
#include "utils/common_util.h"
#include "sql_executor.h"
#include <iostream>
#include "enums/operator_type.h"

template<class Model>
class SqlPage {
public:
    SqlPage(std::string &sql_operator, int page, int page_size)
            : sql_operator_(sql_operator), page_(page), page_size_(page_size) {
        std::cout << "SqlPage init" << std::endl;
        if (page_ <= 0) {
            page_ = 1;
        }
        if (page_size_ <= 0) {
            page_size_ = 10;
        }
        page_size_ = page_size;
        int offset = (page_ - 1) * page_size_;
        this->limit(page_size_);
        this->offset(offset);
    }

    ~SqlPage() {
        std::cout << "SqlPage destroy" << std::endl;
    }

    SqlPage &order_by(std::vector<std::pair<std::string, bool>> &order_by) {
        orderBy_ = "ORDER BY ";
        for (size_t i = 0, len = order_by.size(); i < len; ++i) {
            const auto &item = order_by[i];
            if (i < len - 1) {
                orderBy_ += item.first + (item.second ? " DESC" : " ASC") + ",";
            } else {
                orderBy_ += item.first + (item.second ? " DESC" : " ASC");
            }
        }
        return *this;
    }

    SqlPage &order_by() {
        orderBy_ = "ORDER BY " + ModelTraits<Model>::instance().primary_key() + " DESC";
        return *this;
    }

    std::unique_ptr<SqlExecutor<Model>> operator_sql() {
        std::string countSql(this->sql_operator_);
        CommonUtil::replace_between(countSql, "SELECT", "FROM", " COUNT(*) ");
        return std::make_unique<SqlExecutor<Model>>(
                OperatorType::COUNT, page_,
                page_size_, std::move(countSql), std::move(this->sql_operator_));

    }

protected:
    std::string &querySql() {
        if (!orderBy_.empty()) {
            this->sql_operator_ += " " + orderBy_;
        }
        if (!limit_.empty()) {
            this->sql_operator_ += " " + limit_;
        }
        if (!offset_.empty()) {
            this->sql_operator_ += " " + offset_;
        }
        return this->sql_operator_;
    }

    void limit(int count) {
        limit_ = "LIMIT " + std::to_string(count);
    }

    void offset(int offset) {
        offset_ = "OFFSET " + std::to_string(offset);
    }

private:

    std::string sql_operator_;
    int page_;
    int page_size_;
    std::string limit_;
    std::string offset_;
    std::string orderBy_;
};

#endif
