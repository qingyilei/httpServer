#include "handlers/order/order_delete_handler.h"
#include "model/order.h"
#include <iostream>

std::string OrderDeleteHandler::handle(const http_request &request)  {
    // 模拟实现：打印接收到的body并返回一个成功的消息
    Order order = Order(request.request_body_field<int>("id"));
    order.remove();
    // 创建表（示例）
    return generate_resp<std::string>(std::format("remove id {} success",order.id));
}