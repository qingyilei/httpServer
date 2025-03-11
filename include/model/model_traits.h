//
// Created by qing on 25-3-5.
//


#ifndef MODEL_TRAITS_H
#define MODEL_TRAITS_H


#include <boost/json.hpp>
#include <string>
#include <vector>
#include "meta_data.h"
namespace json = boost::json;

template<typename Model>
class ModelTraits {
public:
    void set_table_name(const std::string &table_name) { table_name_ = table_name; }

    void set_primary_key(const std::string &primary_key) { primary_key_ = primary_key; }

    // 获取字段信息
    const auto &fields() const { return fields_; }

    [[nodiscard]] const std::string& table_name() const { return table_name_; }

    [[nodiscard]] const std::string& primary_key() const { return primary_key_; }

    [[nodiscard]] const std::string& foreign_key() const { return foreign_key_; }

    // 获取单例实例
    static ModelTraits &instance() {
        static ModelTraits instance;
        return instance;
    }

    template<typename T>
    void register_field(const std::string &name,
                        std::function<std::any(const Model &)> getter,
                        std::function<void(Model &, std::any)> setter) {
        fields_[name] =
                FieldMeta<Model>(std::type_index(typeid(T)),getter,setter);
    }


    // 生成建表DDL
    [[nodiscard]]
    std::string generate_create_table_ddl() const {
        std::string sql = "CREATE TABLE IF NOT EXISTS " + table_name_ + " (";
        for (const auto &[name, meta]: fields_) {
            sql += name + " " + type_to_sql(meta.typeIndex) + ", ";
        }
        sql += "PRIMARY KEY(" + primary_key_ + "));";
        return sql;
    }



    template<typename T>
    void set_field(const std::string &name, Model &model, T value) {
        if (fields_.count(name) > 0) {
            fields_[name].setter(model, std::any(value));
        }
    }

    std::any get_field(const std::string &name, Model &model) const {
        if (fields_.count(name) > 0) {
           return fields_.at(name).getter(model); // 需要确保key存在，否则抛出异常
        }
        return nullptr;
    }

private:
    std::string table_name_;
    std::string primary_key_;
    std::string foreign_key_;
    std::unordered_map<std::string, FieldMeta<Model>> fields_;

    // 类型到SQL类型的映射
    static std::string type_to_sql(const std::type_index &type) {
        static const std::unordered_map<std::type_index, std::string> type_map = {
                {typeid(int),                  "INTEGER"},
                {typeid(double),               "REAL"},
                {typeid(std::string),          "TEXT"},
                {typeid(std::vector<uint8_t>), "BLOB"}
        };
        return type_map.at(type);
    }
};

#endif

