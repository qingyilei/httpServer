#include "handlers/user/user_delete_handler.h"
#include "model/user.h"
#include <iostream>
#include <boost/json/serialize.hpp>

std::string UserDeleteHandler::handle(const HttpRequest &request)  {
    // 模拟实现：打印接收到的body并返回一个成功的消息
    User user = User(request.get_json_field<int>("id"));
    user.remove();
    // 创建表（示例）
    return generate_resp<std::string>(std::format("remove id {} success",user.id));
}