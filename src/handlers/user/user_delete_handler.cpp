#include "handlers/user/user_delete_handler.h"
#include "model/user.h"
#include <iostream>

std::string UserDeleteHandler::handle(const http_request &request)  {
    // 模拟实现：打印接收到的body并返回一个成功的消息
    User user = User(request.request_body_field<int>("id"));
    user.remove();
//
//    SqlOperator<User>::instance().remove()->where()
//            ->eq<int>("id",user).operator_sql()
//            ->remove_execute(user);
    // 创建表（示例）
    return generate_resp<std::string>(std::format("remove id {} success",user.id));
}