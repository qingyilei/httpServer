//
// Created by qing on 2025/3/5.
//


#ifndef MODEL_H
#define MODEL_H

#include <boost/json.hpp>
#include <variant>
#include <unordered_map>
#include "database/database.h"
#include "enums/message_level.h"
#include "utils/stream_container.h"
#include <iostream>
#include "database/query_builder.h"
#include <ranges>
#include <format>
#include <numeric>

namespace json = boost::json;

// 字段类型定义
using FieldValue = std::variant<
        int,
        double,
        std::string,
        std::vector<uint8_t>,
        std::nullptr_t
>;

// 模型元信息
template<typename Derived>
struct ModelTraits;

// 模型基类 (CRTP)
template<typename Derived>
class Model {
public:
    // 字段访问器
    template<typename T>
    T get(const std::string &field_name) const {
        return std::get<T>(fields_.at(field_name));
    }

    // 字段设置器
    template<typename T>
    void set(const std::string &field_name, T value) {
        fields_[field_name] = value;
    }

    virtual std::vector<std::pair<MessageLevel, std::string>> verify() = 0;

    [[nodiscard]] virtual json::object to_json() const = 0;

    QueryBuilder<Derived> builder() {
        return QueryBuilder<Derived>(*static_cast<Derived *>(this));
    }

    void save() {
        std::vector<std::pair<MessageLevel, std::string>> verify_result = this->verify();
        if (!verify_result.empty()) {
            auto processed = verify_result
                             | std::views::filter([](const std::pair<MessageLevel, std::string> &u) { // 过滤条件
                return u.first == MessageLevel::ERROR;
            })
                             | std::views::transform([](const std::pair<MessageLevel, std::string> &u) { // 转换操作
                return u.second;
            });
            std::string errorLines = std::accumulate(std::ranges::begin(processed),
                                                 std::ranges::end(processed),
                                                 std::string{}, [](const std::string &a, const std::string &b) {
                        return a.empty() ? b : a + "\n" + b;
                    });
            throw std::runtime_error(errorLines);
        }

        auto &db = Database::instance();
        const std::string &id = ModelTraits<Derived>::instance().primary_key();
        int count = this->fields_.count(id);
        int saveId = (count == 0 || get<int>("id") == 0) ?
                     db.insert(*static_cast<Derived *>(this))
                                                         : db.update(*static_cast<Derived *>(this));
        this->set("id", saveId);

    }

    void remove() {
        Database::instance().remove(*static_cast<Derived *>(this));
    }

    [[nodiscard]] FieldValue get_field_value(const std::string &field_name) const {
        if (fields_.count(field_name) == 0) {
            throw std::invalid_argument("Field not found");
        }
        return fields_.at(field_name);
    }

    // 关联关系
    template<typename Related>
    auto has_many() {
        return Relation<Derived, Related>(
                ModelTraits<Derived>::instance().primary_key(),
                ModelTraits<Related>::foreign_key()
        );
    }

    template<typename T>
    static void base_register_field(ModelTraits<Derived> &traits, const std::string &name, T Derived::* member) {
        traits.template register_field<T>(name,
                                          [member](const Derived &u) -> std::any { return u.*member; },
                                          [member](Derived &u, const std::any &value) {
                                              if (value.type() == typeid(T)) {
                                                  u.*member = std::any_cast<T>(value);
                                              } else {
                                                  throw std::invalid_argument("Type mismatch in register_field");
                                              }
                                          });
    }

protected:
    std::unordered_map<std::string, FieldValue> fields_;
};

#endif

