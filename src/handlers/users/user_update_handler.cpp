#include "handlers/user/user_update_handler.h"
#include <sstream>
#include "model/user.h"

std::string UserUpdateHandler::handle(const HttpRequest &request) {
    // 模拟实现：打印接收到的body并返回一个成功的消息

    User user = User(request.get_json_field<int>("id"),
                     request.get_json_field<std::string>("name"),
                     request.get_json_field<std::string>("email"),
                     request.get_json_field<int>("age"));
    user.save();
    return normal_rvalue_response(user.to_json());
}