//
// Created by qing on 25-3-6.
//
#include "model/user.h"

#include "model/model_traits.h"

void User::register_fields() {
    auto &traits = ModelTraits<User>::instance();
    traits.set_table_name("user");
    traits.set_primary_key("id");
    User::base_register_field(traits, "id", &User::id);
    User::base_register_field<int>(traits, "id", &User::id);
    User::base_register_field<std::string>(traits, "name", &User::name);
    User::base_register_field<int>(traits, "age", &User::age);
    User::base_register_field<std::string>(traits, "email", &User::email);
}

std::vector<std::pair<MessageLevel,std::string>>  User::verify() {
    return {};
}



