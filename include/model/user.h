//
// Created by qing on 2025/3/5.
//

#ifndef USER_H
#define USER_H

#include "model_traits.h"
#include "model.h"

class User : public Model<User> {
public:
    User(int id,  std::string name,  std::string email, int age)
    : id(id),name(std::move(name)), email(std::move(email)),age(age) {

        set("id", id);
        set("age",age);
        set<std::string>("name", std::move(name));
        set<std::string>("email", std::move(email));
    }

    User(int id) : id(id),age(0) {
        set<int>("id", std::move(id));
    }
    User() = default;

    std::vector<std::pair<MessageLevel,std::string>>  verify() override;

    int id;
    std::string name;
    std::string email;
    int age;

    static void register_fields();

    [[nodiscard]] boost::json::object to_json() const override {
        return json::object{
                {"id",    id},
                {"name",  name},
                {"email", email},
                {"age",   age}
        };
    }

};


#endif //USER_H