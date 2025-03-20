#include "handlers/order/order_user_query_handler.h"
#include "model/order_user.h"
#include "model/user.h"
#include "utils/common_util.h"
#include <sstream>

std::string OrderUserQueryHandler::handle(const http_request &request) {
    auto &params = request.params;
    int id = CommonUtil::get_param(params, "id", 0);
    std::vector<int> orderNos = CommonUtil::get_all_params<int>(params, "orderNo");
    int page = CommonUtil::get_param(params, "page", 1);
    int size = CommonUtil::get_param(params, "pageSize", 10);
    std::vector<std::pair<std::string, bool>> orderBy = {std::make_pair("orders_.id", true)};
    auto result = SqlOperator<OrderUser>::instance().select()
            ->relation<User>("create_by", "id").fields()
            ->field<User>("name", "create_name").where()
            ->in_condition<int>("order_no", orderNos)
            .page(page, size)
            ->order_by(orderBy).operator_sql()->query_execute();

    return generate_resp(result.to_json());
}