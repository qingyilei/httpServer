#include "handlers/order/order_batch_handler.h"
#include <string>
#include "model/order.h"
#include "sql/sql_operator.h"

std::string OrderBatchHandler::handle(const http_request &request) {

    Order order = Order(0,
                        request.request_body_field<int>("order_no"),
                        request.request_body_field<int>("order_qty"),
                        request.request_body_field<double>("unit_price"),
                        request.request_body_field<int>("create_by"),
                        request.request_body_field<std::string>("create_time"));

    std::vector<Order> orders;
    for (int i = 0; i < 50; ++i) {
        order.set("order_no", (i + 53));
        order.order_no = (53 + i);
        orders.push_back(order);
    }
    int size = SqlOperator<Order>::instance().insert()
            ->fields()
            ->clear_field("id")
            .operator_sql()->batch_execute(orders);

    return generate_resp<std::string>(std::format("batch size {} success", size));

}