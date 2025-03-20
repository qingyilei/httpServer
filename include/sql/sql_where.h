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
    explicit SqlWhere(std::string &&sql_operator) : sql_operator_(sql_operator) {
        if (!sql_operator.empty() && CommonUtil::contains(sql_operator, "JOIN")) {
            source_alias_ = ModelTraits<Model>::instance().table_name() + "_";
        }
        std::cout << "SqlWhere init" << std::endl;
    }

    ~SqlWhere() {
        std::cout << "SqlWhere destroy" << std::endl;
    }

    template<typename T>
    SqlWhere<Model> &eq(const std::string &field, Model &model_) {
        auto value = ModelTraits<Model>::instance().get_field(field, model_);
        put_condition(field,std::any_cast<T>(value));
        return *this;
    }

    template<typename T>
    SqlWhere<Model> &in_condition(const std::string &field, const std::vector<T> &in_vals) {
        std::string in_condition = CommonUtil::join_with_comma(in_vals);
        if (in_condition.empty() || std::all_of(in_condition.begin(), in_condition.end(), ::isspace)) {
            return *this;
        }
        put_in_condition(field,in_condition);
        return *this;
    }


    SqlWhere<Model> &like_condition(const std::string &field, Model &model_) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} like '%{}%'", get_field(field), std::any_cast<std::string>(val)));
        return *this;
    }

    SqlWhere<Model> &l_like_condition(const std::string &field, Model &model_) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} like '%{}'", get_field(field), std::any_cast<std::string>(val)));
        return *this;
    }

    SqlWhere<Model> &r_like_condition(const std::string &field, Model &model_) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        sql_condition_.push_back(std::format("{} like '{}%'", get_field(field), std::any_cast<std::string>(val)));
        return *this;
    }

    std::unique_ptr<SqlPage<Model>> page(int page, int page_size) {
        std::string condition = join_conditions(sql_condition_);
        if (condition.empty()) {
            CommonUtil::replace_all(this->sql_operator_, "%w", "1=1");
        } else {
            CommonUtil::replace_all(this->sql_operator_, "%w", condition);
        }
        return std::make_unique<SqlPage<Model>>(this->sql_operator_, page, page_size);
    }

    std::unique_ptr<SqlExecutor<Model>> operator_sql() {

        std::string condition = join_conditions(sql_condition_);
        if (condition.empty()) {
            CommonUtil::replace_all(this->sql_operator_, "%w", "1=1");
        } else {
            CommonUtil::replace_all(this->sql_operator_, "%w", condition);
        }
        CommonUtil::replace_all(this->sql_operator_, "%w", condition);
        return std::make_unique<SqlExecutor<Model>>(
                OperatorType::CREATE, 0,
                0, "", std::move(this->sql_operator_));
    }

private:

    std::string get_field(const std::string &field) {
        return source_alias_.empty() ? field : source_alias_ + "." + field;
    }

    template<typename T>
    void put_condition(const std::string &field, T value) {
        if (this->source_alias_.empty()) {
            sql_condition_.push_back(std::format("{} = {}", field, value));
        } else {
            sql_condition_.push_back(std::format("{} = {}", this->source_alias_ + "." + field, value));
        }
    }

    void put_in_condition(const std::string &field,const std::string &value) {
        if (this->source_alias_.empty()) {
            sql_condition_.push_back(std::format("{} in ({})", field, value));
        } else {
            sql_condition_.push_back(std::format("{} in ({})", this->source_alias_ + "." + field, value));
        }
    }

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
    std::string source_alias_;
};


#endif
