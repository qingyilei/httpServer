//
// Created by qing on 25-3-6.
//
#include <string>
#include "database/query_builder.h"
#include "database/database.h"

#ifndef HTTP_SERVER_RELATION_H
#define HTTP_SERVER_RELATION_H
// 关联关系模板
template<typename Owner, typename Target>
class Relation {
public:
    Relation(std::string owner_key, std::string target_key)
            : owner_key_(std::move(owner_key)),
              target_key_(std::move(target_key)) {}

    QueryBuilder<Target> query() const {
        auto owner_id = Owner::current().template get<int>(owner_key_);
        return QueryBuilder<Target>(Database::instance())
                .where(target_key_ + " = ?")
                .bind(owner_id);
    }

private:
    std::string owner_key_;
    std::string target_key_;
};

#endif //HTTP_SERVER_RELATION_H
