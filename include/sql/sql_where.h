//
// Created by qing on 2025/3/5.
//

#ifndef SQL_WHERE_H
#define SQL_WHERE_H

#include <string>
#include <vector>
#include "model/model_traits.h"
#include "sql_page.h"
#include <sstream>
#include <iostream>


template<class Model>
class SqlWhere {
public:
    /**
     * model traits table_name
     * @param table
     */
    explicit SqlWhere(std::string &sql_operator) : sql_operator_(sql_operator) {
        std::cout << "SqlWhere init" << std::endl;
    }

    ~SqlWhere() {
        std::cout << "SqlWhere destroy" << std::endl;
    }

    template<typename T>
    SqlWhere<Model> &eq(const std::string &field, Model &model_) {
        auto value = ModelTraits<Model>::instance().get_field(field, model_);
        sql_condition_.push_back(std::format("{} = {}", field, std::any_cast<T>(value)));
        return *this;
    }

    template<typename T>
    SqlWhere<Model> &in_condition(const std::string &field, const std::vector<T> &in_vals) {
        std::string in_condition = CommonUtil::join_with_comma(in_vals);
        if (in_condition.empty() || std::all_of(in_condition.begin(), in_condition.end(), ::isspace)) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} in ({})", field, in_condition));
        return *this;
    }


    SqlWhere<Model> &like_condition(const std::string &field, Model &model_) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} like '%{}%'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    SqlWhere<Model> &l_like_condition(const std::string &field, Model &model_) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} like '%{}'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    SqlWhere<Model> &r_like_condition(const std::string &field, Model &model_) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} like '{}%'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    std::unique_ptr<SqlPage<Model>> page(int page, int page_size) {
        CommonUtil::replace_all(this->sql_operator_, "%w", join_conditions(sql_condition_));
        return std::make_unique<SqlPage<Model>>(this->sql_operator_,page,page_size);
    }

    std::unique_ptr<SqlExecutor<Model>> operator_sql() {

        auto count_sql = std::string("");
        CommonUtil::replace_all(this->sql_operator_, "%w", join_conditions(sql_condition_));
        return std::make_unique<SqlExecutor<Model>>(
                OperatorType::CREATE, 0,
                0, count_sql, this->sql_operator_);
    }

private:
    std::string join_conditions(std::vector<std::string> &where_) {
        std::string result;
        for (size_t i = 0; i < where_.size(); ++i) {
            if (i > 0) result += " AND ";
            result += "(" + where_[i] + ")";
        }
        return result;
    }
    std::vector<std::string> sql_condition_;
    std::string sql_operator_;
};


#endif
