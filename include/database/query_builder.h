//
// Created by qing on 2025/3/5.
//

#ifndef QUERY_BUILDER_H
#define QUERY_BUILDER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <functional>
#include <boost/format.hpp>
#include "query_result.h"
#include "database.h"
#include "model/model_traits.h"
#include "utils/convert_util.h"
#include <sstream>
#include <iostream>

template<typename Model>
class QueryBuilder {
public:

    explicit QueryBuilder(Model &model) : model_(model),page_size_(20),page_(1) {
    }

    // 新增方法：根据Model对象自动组装where条件
    QueryBuilder<Model> &where(const Model &model) {
        const auto &traits = ModelTraits<Model>::instance();
        for (const auto &[name, field]: traits.fields()) {
            auto value = model.get_field_value(name);
            std::visit([&](auto &&val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, int>) {
                    where_clauses_.push_back(std::format("{} = {}", name, val));
                } else if constexpr (std::is_same_v<T, double>) {
                    where_clauses_.push_back(std::format("{} = {}", name, val));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    where_clauses_.push_back(std::format("{} = '{}'", name, val));
                } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                    // 对于二进制数据，暂时不支持直接比较
                    throw std::runtime_error("Unsupported type for where clause");
                }
            }, value);
        }
        return *this;
    }

    QueryBuilder<Model> &and_condition(const std::string &condition) {
        where_clauses_.push_back(condition);
        return *this;
    }

    QueryBuilder<Model> &eq_condition(const std::string &field) {
        auto value = ModelTraits<Model>::instance().get_field(field);
        where_clauses_.push_back(std::format("{} = {}", field, value));
        return *this;
    }

    template<typename T>
    QueryBuilder<Model> &in_condition(const std::string &field, const std::vector<T> &in_vals) {
        std::string in_condition = vector_to_string(in_vals);
        if (in_condition.empty() || std::all_of(in_condition.begin(), in_condition.end(), ::isspace)) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} in ({})", field, in_condition));
        return *this;
    }


    QueryBuilder<Model> &like_condition(const std::string &field) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} like '%{}%'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    QueryBuilder<Model> &l_like_condition(const std::string &field) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} like '%{}'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    QueryBuilder<Model> &r_like_condition(const std::string &field) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} like '{}%'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    QueryBuilder<Model> &order_by(const std::string &column, bool ascending) {
        order_ = "ORDER BY " + column + (ascending ? " ASC" : " DESC");
        return *this;
    }

    QueryBuilder<Model> &page(int page) {
        page_ = page;
        return *this;
    }

    QueryBuilder<Model> &page_size(int size) {
        page_size_ = size;
        return *this;
    }


    QueryResult<Model> execute() {
        int offset = (page_ - 1) * page_size_;
        this->limit(page_size_);
        this->offset(offset);

        const std::pair<std::string, std::string> query_count_sql = build_query();

        auto &db = Database::instance();
        int total = db.query_count(query_count_sql.first);
        auto result = db.query_data<Model>(query_count_sql.second);

        return result.with_metadata({
                                            page_,
                                            page_size_,
                                            total,
                                            (total + page_size_ - 1) / page_size_
                                    });
    }


    [[nodiscard]] std::pair<std::string, std::string> build_query() const {

        std::string count_sql = "SELECT count(*) FROM "
                                + ModelTraits<Model>::instance().table_name();

        std::string query_sql = "SELECT * FROM "
                                + ModelTraits<Model>::instance().table_name();


        if (!where_clauses_.empty()) {
            std::string join_str = " WHERE " + join_conditions();
            count_sql += join_str;
            query_sql += join_str;
        }
        if (!order_.empty()) {
            query_sql += " " + order_;
        }

        if (!limit_.empty()) {
            query_sql += " " + limit_;
        }
        if (!offset_.empty()) {
            query_sql += " " + offset_;
        }
        return std::make_pair(count_sql, query_sql);
    }

private:
    QueryBuilder<Model> &limit(int count) {
        limit_ = "LIMIT " + std::to_string(count);
        return *this;
    }

    QueryBuilder<Model> &offset(int offset) {
        offset_ = "OFFSET " + std::to_string(offset);
        return *this;
    }



    [[nodiscard]] std::string join_conditions() const {
        std::string result;
        for (size_t i = 0; i < where_clauses_.size(); ++i) {
            if (i > 0) result += " AND ";
            result += "(" + where_clauses_[i] + ")";
        }
        return result;
    }

    Model model_;
    std::vector<std::string> where_clauses_;
    std::string order_;
    std::string limit_;
    std::string offset_;
    int page_;
    int page_size_;
};

// 其他模板方法实现...
#endif //QUERY_BUILDER_H
