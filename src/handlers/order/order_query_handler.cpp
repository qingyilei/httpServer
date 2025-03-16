#include "handlers/order/order_query_handler.h"
#include "model/order.h"
#include "utils/common_util.h"
#include <sstream>

std::string OrderQueryHandler::handle(const http_request &request) {
    auto &params = request.params;
    int id = CommonUtil::get_param(params, "id", 0);
    std::vector<int> orderNos = CommonUtil::get_all_params<int>(params, "orderNo");
    int page = CommonUtil::get_param(params, "page", 1);
    int size = CommonUtil::get_param(params, "pageSize", 10);
    auto result = SqlOperator<Order>::instance().select()
            ->fields()
            ->where()->in_condition<int>("order_no", orderNos)
            .page(page, size)
            ->order_by().operator_sql()->query_execute();

    return generate_resp(result.to_json());
}