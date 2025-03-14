//
// Created by qing on 2025/3/5.
//

#ifndef SQL_TABLE_H
#define SQL_TABLE_H
#include <string>
#include <vector>
#include <functional>
#include "sql_field.h"
#include <iostream>

template<class Model>
class SqlTable {
public:
    /**
     * model traits table_name
     * @param table
     */
    SqlTable(std::string operator_sql,const std::string &table) : operator_sql_(std::move(operator_sql)), sql_table_(table) {
        std::cout << "SqlTable init" << std::endl;
    }

    ~SqlTable() {
        std::cout << "SqlTable destroy" << std::endl;
    }
   SqlTable &table(const std::string &table) {
        this->sql_table_ = table;
       return *this;
    }

    std::unique_ptr<SqlField<Model>> fields() {
        CommonUtil::replace_all(this->operator_sql_, "%t", sql_table_);
        std::vector<std::string> fields = ModelTraits<Model>::instance().model_fields();
        return std::make_unique<SqlField<Model>>(this->operator_sql_,fields);
    }


private:
    std::string operator_sql_;
    std::string sql_table_;
};


#endif
