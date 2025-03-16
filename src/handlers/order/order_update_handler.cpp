#include "handlers/order/order_update_handler.h"
#include <sstream>
#include "model/order.h"

std::string OrderUpdateHandler::handle(const http_request &request) {
    // 模拟实现：打印接收到的body并返回一个成功的消息

    Order order = Order(request.request_body_field<int>("id"),
                        request.request_body_field<int>("order_no"),
                        request.request_body_field<int>("order_qty"),
                        request.request_body_field<double>("unit_price"),
                        request.request_body_field<int>("create_by"),
                        request.request_body_field<std::string>("create_time")
    );
    order.save();
    return normal_rvalue_response(order.to_json());
}