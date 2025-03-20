//
// Created by qing on 2025/3/5.
//

#ifndef SQL_TABLE_H
#define SQL_TABLE_H
#include <string>
#include <utility>
#include <vector>
#include <functional>
#include "sql_field.h"
#include <iostream>
#include "sql/relation.h"

template<class Model>
class SqlTable {
public:
    /**
     * model traits table_name
     * @param table
     */
    SqlTable(std::string &&operator_sql,std::string table) : operator_sql_(operator_sql), sql_table_(std::move(table)) {
        std::cout << "SqlTable init" << std::endl;
    }

    ~SqlTable() {
        std::cout << "SqlTable destroy" << std::endl;
    }
   SqlTable &table(const std::string &table) {
        this->sql_table_ = table;
       return *this;
    }

    template<class Target>
    SqlTable &relation(const std::string &owner_key,const std::string &target_key) {
        this->sql_table_ = Relation<Model,Target>(owner_key,target_key).build_relation();
        return *this;
    }


    std::unique_ptr<SqlField<Model>> fields() {
        CommonUtil::replace_all(this->operator_sql_, "%t", sql_table_);
        std::vector<std::string> fields = ModelTraits<Model>::instance().model_fields();
        return std::make_unique<SqlField<Model>>(std::move(this->operator_sql_),fields);
    }

    std::unique_ptr<SqlWhere<Model>> where() {
        CommonUtil::replace_all(this->operator_sql_, "%t", sql_table_);
        return std::make_unique<SqlWhere<Model>>(std::move(this->operator_sql_));
    }

    std::string operator_sql() {
        return this->operator_sql_;
    }

    std::string sql_table() {
        return this->sql_table_;
    }

private:
    std::string operator_sql_;
    std::string sql_table_;
};


#endif
