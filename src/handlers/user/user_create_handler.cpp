#include "handlers/user/user_create_handler.h"
#include <string>
#include "model/user.h"

std::string UserCreateHandler::handle(const HttpRequest &request) {


    User user = User(0,
                     request.request_body_field<std::string>("name"),
                     request.request_body_field<std::string>("email"),
                     request.request_body_field<int>("age"));

    user.save();
    return normal_rvalue_response(user.to_json());
}