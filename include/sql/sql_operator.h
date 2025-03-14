//
// Created by qing on 2025/3/5.
//

#ifndef SQL_OPERATOR_H
#define SQL_OPERATOR_H

#include <string>
#include <vector>
#include "sql_table.h"

template<class Model>
class SqlOperator {
public:
    static SqlOperator &instance(){
        static SqlOperator instance_;
        return instance_;
    }
    ~SqlOperator() {
        std::cout << "SqlOperator destroy" << std::endl;
    }

    std::unique_ptr<SqlTable<Model>> update() {
        return std::make_unique<SqlTable<Model>>(std::string( "UPDATE %t SET %f WHERE %w "), ModelTraits<Model>::instance().table_name());
    }

    std::unique_ptr<SqlTable<Model>> select() {
        std::string sql = std::string( "SELECT %f FROM %t WHERE %w ");
        return std::make_unique<SqlTable<Model>>(std::string( "SELECT %f FROM %t WHERE %w "),
                                                 ModelTraits<Model>::instance().table_name());
    }

    std::unique_ptr<SqlTable<Model>> remove() {
        return std::make_unique<SqlTable<Model>>(std::string( "DELETE FROM %t WHERE %w "), ModelTraits<Model>::instance().table_name());

    }

    std::unique_ptr<SqlTable<Model>> insert() {
        return std::make_unique<SqlTable<Model>>(std::string( "INSERT INTO %t (%f) values (%v)"),
                                          ModelTraits<Model>::instance().table_name());
    }
private:
    SqlOperator() {
        std::cout << "SqlOperator init" << std::endl;
    };
};

#endif
