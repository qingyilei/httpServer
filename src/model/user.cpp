//
// Created by qing on 25-3-6.
//
#include "model/user.h"

#include "model/model_traits.h"

template<typename T>
void User::register_field(ModelTraits<User> &traits, const std::string &name, T User::* member) {
    traits.register_field<T>(name,
                             [member](const User &u) -> std::any { return u.*member; },
                             [member](User &u, const std::any &value) {
                                 if (value.type() == typeid(T)) {
                                     u.*member = std::any_cast<T>(value);
                                 } else {
                                     throw std::invalid_argument("Type mismatch in register_field");
                                 }
                             });
}

void User::register_fields() {
    auto &traits = ModelTraits<User>::instance();
    traits.set_table_name("user");
    traits.set_primary_key("id");
    register_field(traits, "id", &User::id);
    User::base_register_field<int>(traits, "id", &User::id);
    User::register_field<std::string>(traits, "name", &User::name);
    User::register_field<int>(traits, "age", &User::age);
    User::register_field<std::string>(traits, "email", &User::email);
}



