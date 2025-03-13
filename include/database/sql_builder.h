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
#include <numeric>

std::string joinWith(std::vector<std::string> &processed, const std::string &delimiter) {

    std::string joinLine = std::accumulate(std::ranges::begin(processed),
                                             std::ranges::end(processed),
                                             std::string{}, [delimiter](const std::string &a, const std::string &b) {
                return a.empty() ? b : a + delimiter + b;
            });
    return joinLine;
}


class SqlWhere {
public:
    /**
     * model traits table_name
     * @param table
     */
    SqlWhere(std::vector<std::string> &condition) : sql_condition_(condition) {
        std::cout << "SqlTable init" << std::endl;
    }

    ~SqlWhere() {
        std::cout << "SqlTable destroy" << std::endl;
    }

private:
    std::vector<std::string> sql_condition_;
};

class SqlTable {
public:
    /**
     * model traits table_name
     * @param table
     */
    SqlTable(std::string &table) : sql_table_(table) {
        std::cout << "SqlTable init" << std::endl;
    }

    ~SqlTable() {
        std::cout << "SqlTable destroy" << std::endl;
    }

private:
    std::string sql_table_;
};

template<class Model>
class SqlField {
public:
    SqlField(std::vector<std::string> &fields) : fields_(fields) {
        std::cout << "SqlField init" << std::endl;
    }

    SqlField &field(const std::string &field) {
        this->fields_.push_back(field);
        return *this;
    }

    ~SqlField() {
        std::cout << "SqlFeild destroy" << std::endl;
    }

    std::unique_ptr<SqlTable> table() {
        return std::make_unique<SqlTable>(ModelTraits<Model>::instance().table_name());
    }

    std::unique_ptr<SqlTable> table(const std::string &table) {
        return std::make_unique<SqlTable>(table);
    }


private:
    /**
     * 1. select|update|delete
     * 2. select or update fields
     * 3. form table
     * 4. where condition
     */
    std::vector<std::string> fields_;
    std::vector<std::string> sql_vector_;
};


template<typename Model>
class SqlBuilder {
public:

    explicit SqlBuilder(Model &model) : model_(model), page_size_(20), page_(1) {
    }


    SqlBuilder<Model> &where(const std::string &condition) {
        where_clauses_.push_back(condition);
        return *this;
    }


    // 新增方法：根据Model对象自动组装where条件
    SqlBuilder<Model> &where(const Model &model) {
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

    SqlBuilder<Model> &and_condition(const std::string &condition) {
        where_clauses_.push_back(condition);
        return *this;
    }

    template<typename T>
    SqlBuilder<Model> &eq_condition(const std::string &field) {
        auto value = ModelTraits<Model>::instance().get_field(field, model_);
        where_clauses_.push_back(std::format("{} = {}", field, std::any_cast<T>(value)));
        return *this;
    }

    template<typename T>
    SqlBuilder<Model> &in_condition(const std::string &field, const std::vector<T> &in_vals) {
        std::string in_condition = vector_to_string(in_vals);
        if (in_condition.empty() || std::all_of(in_condition.begin(), in_condition.end(), ::isspace)) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} in ({})", field, in_condition));
        return *this;
    }


    SqlBuilder<Model> &like_condition(const std::string &field) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} like '%{}%'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    SqlBuilder<Model> &l_like_condition(const std::string &field) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} like '%{}'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    SqlBuilder<Model> &r_like_condition(const std::string &field) {
        auto val = ModelTraits<Model>::instance().get_field(field, model_);
        if (!val.has_value() || std::any_cast<std::string>(val).empty()) {
            return *this;
        }
        where_clauses_.push_back(std::format("{} like '{}%'", field, std::any_cast<std::string>(val)));
        return *this;
    }

    SqlBuilder<Model> &order_by(const std::string &column, bool ascending) {
        order_ = "ORDER BY " + column + (ascending ? " ASC" : " DESC");
        return *this;
    }

    SqlBuilder<Model> &page(int page) {
        page_ = page;
        return *this;
    }

    SqlBuilder<Model> &page_size(int size) {
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

    int execute_count() {

        const std::pair<std::string, std::string> query_count_sql = build_query();
        auto &db = Database::instance();
        std::cout << "cout sql;" << query_count_sql.first << std::endl;
        return db.query_count(query_count_sql.first);

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
    SqlBuilder<Model> &limit(int count) {
        limit_ = "LIMIT " + std::to_string(count);
        return *this;
    }

    SqlBuilder<Model> &offset(int offset) {
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
