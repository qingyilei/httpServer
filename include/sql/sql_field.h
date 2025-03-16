//
// Created by qing on 2025/3/5.
//

#ifndef SQL_FIELD_H
#define SQL_FIELD_H

#include <string>
#include <vector>
#include "utils/common_util.h"
#include "sql_where.h"
#include "log/logger.h"


template<class Model>
class SqlField {
public:
    SqlField(std::string &&operator_sql, std::vector<std::string> &fields) : operator_sql_(operator_sql),
                                                                            fields_(fields) {
        std::cout << "SqlField init" << std::endl;
    }

    ~SqlField() {
        std::cout << "SqlField destroy" << std::endl;
    }

    SqlField &field(const std::string &field) {
        this->fields_.push_back(field);
        return *this;
    }


    SqlField &clear_field(const std::string &field) {

        // 使用 std::remove_if + lambda谓词，并配合 erase 删除元素
        this->fields_.erase(
                std::remove_if(this->fields_.begin(), this->fields_.end(),
                               [&](const std::string &elem) {
                                   return elem == field; // 直接比较字符串
                               }),
                this->fields_.end()
        );
        return *this;
    }

    SqlField &clear_fields() {
        this->fields_.clear();
        return *this;
    }

    std::unique_ptr<SqlWhere<Model>> where() {
        std::string join_fields = replace_fields();
        CommonUtil::replace_all(this->operator_sql_, "%f", join_fields);
        return std::make_unique<SqlWhere<Model>>(std::move(this->operator_sql_));
    }


    std::unique_ptr<SqlExecutor<Model>> operator_sql() {
        replace_fields();
        if (this->operator_sql_.find("INSERT") != std::string::npos) {
            // 检查 fields_ 是否为空
            if (this->fields_.empty()) {
                // 根据业务需求处理，例如抛出异常或返回错误
                // 当前代码保留原有行为，但需注意此处逻辑风险
                Logger::getInstance().error("INSERT operation requires non-empty fields");
                throw std::runtime_error("INSERT operation requires non-empty fields");

            }
            std::ostringstream val_stream;
            const size_t len = this->fields_.size();
            for (size_t i = 0; i < len; ++i) {
                val_stream << ":" << fields_[i];
                if (i < len - 1) {
                    val_stream << ",";
                }
            }
            CommonUtil::replace_all(this->operator_sql_, "%v", val_stream.str());
        }
        return std::make_unique<SqlExecutor<Model>>(
                OperatorType::COUNT, 0,
                0,"", std::move(this->operator_sql_));
    }


private:

    std::string replace_fields() {
        std::string join_fields = CommonUtil::join_with_delimiter(this->fields_, ",");
        if (this->operator_sql_.find("UPDATE") != std::string::npos) {
            const auto &traits = ModelTraits<Model>::instance();
            std::string set_clause;
            for (const auto &[name, _]: traits.fields()) {
                if (name == traits.primary_key()) continue;

                if (!set_clause.empty()) set_clause += ", ";
                set_clause += name + " = :"+name;
            }
            join_fields = set_clause;
        }
        CommonUtil::replace_all(this->operator_sql_, "%f", join_fields);
        return join_fields;
    }
    /**
     * 1. select|update|delete
     * 2. select or update fields
     * 3. form table
     * 4. where condition
     */
    std::vector<std::string> fields_;
    std::string operator_sql_;
};

#endif
