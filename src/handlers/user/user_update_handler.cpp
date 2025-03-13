#include "handlers/user/user_update_handler.h"
#include <sstream>
#include "model/user.h"

std::string UserUpdateHandler::handle(const HttpRequest &request) {
    // 模拟实现：打印接收到的body并返回一个成功的消息

    User user = User(request.request_body_field<int>("id"),
                     request.request_body_field<std::string>("name"),
                     request.request_body_field<std::string>("email"),
                     request.request_body_field<int>("age"));
    user.save();
    return normal_rvalue_response(user.to_json());
}