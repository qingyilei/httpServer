//
// Created by qing on 25-3-6.
//

#ifndef HTTP_SERVER_USER_TRAITS_H
#define HTTP_SERVER_USER_TRAITS_H
#include "model_traits.h"
#include "user.h"
class UserTraits: public ModelTraits<User> {
public:
    void register_fields() {
        set_primary_key("id");
        set_table_name("user");
        regist<int>("id", &User::id);
        regist<std::string>("name", &User::name);
        regist<int>("age", &User::age);
        regist<std::string>("email", &User::email);
    }

    // 获取单例实例
    static UserTraits &instance() {
        static UserTraits instance;
        return instance;
    }


    template<typename T>
    void regist(const std::string &name, T User::* member) {
        register_field<T>(name
                       , typeid(T)
                       , [member](const User &u) { return u.*member; }
                       ,[member](User &u, T value) { u.*member = value; } );
    }
};


#endif //HTTP_SERVER_USER_TRAITS_H
