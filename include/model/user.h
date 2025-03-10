//
// Created by qing on 2025/3/5.
//

#ifndef USER_H
#define USER_H

#include "model_traits.h"
#include "model.h"

class User : public Model<User> {
public:
    User(int id, std::string name, std::string email, int age) : id(id), name(std::move(name)), email(std::move(email)),
                                                                 age(age) {
        set<int>("id", this->id  );
        set<std::string>("name", this->name);
        set<int>("age", this->age       );
        set<std::string>("email", this->email);
    }

    User(int id) : id(id) {
        set<int>("id", this->id);
    }
    User() = default;



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

    template<typename T>
    static void register_field(ModelTraits<User> &traits, const std::string &name, T User::* member);
};


#endif //USER_H