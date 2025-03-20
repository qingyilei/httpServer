//
// Created by qing on 25-3-6.
//

#ifndef HTTP_SERVER_RELATION_H
#define HTTP_SERVER_RELATION_H

#include <string>
#include "model/model_traits.h"
#include <sstream>

// 关联关系模板
template<typename Owner, typename Target>
class Relation {
public:
    Relation(std::string owner_key, std::string target_key)
            : owner_key_(std::move(owner_key)),
              target_key_(std::move(target_key)) {}

    std::string build_relation() const {
        std::string source_table = ModelTraits<Owner>::instance().table_name();
        std::string target_table = ModelTraits<Target>::instance().table_name();
        std::ostringstream sql;
        sql << source_table << " AS " << source_table << "_"
            << " JOIN " << target_table << " AS " << target_table << "_"
            << " ON " << source_table << "_." << owner_key_ << " = " << target_table << "_." << target_key_;
        return sql.str();
    }

private:
    std::string owner_key_;
    std::string target_key_;
};

#endif //HTTP_SERVER_RELATION_H
