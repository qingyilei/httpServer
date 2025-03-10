#include "handlers/user/user_create_handler.h"
#include <iostream>
#include <boost/json/serialize.hpp>
#include <string>
#include "model/user.h"

std::string UserCreateHandler::handle(const HttpRequest &request) {


    User user = User(0,
                     request.get_json_field<std::string>("name"),
                     request.get_json_field<std::string>("email"),
                     request.get_json_field<int>("age"));

    user.save();
    return normal_rvalue_response(user.to_json());
}