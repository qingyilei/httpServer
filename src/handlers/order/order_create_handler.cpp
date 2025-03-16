#include "handlers/order/order_create_handler.h"
#include <string>
#include "model/order.h"

std::string OrderCreateHandler::handle(const http_request &request) {


    Order order = Order(0,
                        request.request_body_field<int>("order_no"),
                        request.request_body_field<int>("order_qty"),
                        request.request_body_field<double>("unit_price"),
                        request.request_body_field<int>("create_by"),
                        request.request_body_field<std::string>("create_time"));
    order.save();
    return normal_rvalue_response(order.to_json());
}