#include "handlers/order/order_query_handler.h"
#include "model/order.h"
#include "utils/convert_util.h"
#include <sstream>

std::string OrderQueryHandler::handle(const HttpRequest &request) {
    auto &params = request.params;
    int id = get_param(params, "id", 0);
    std::vector<int> orderNos = get_all_params<int>(params, "orderNos");
    int page = get_param(params, "page", 1);
    int size = get_param(params, "pageSize", 10);
    Order order;
    auto result = order.builder()
            .like_condition("name")
            .r_like_condition("email")
            .in_condition<int>("order_no", orderNos)
            .order_by("id", false)
            .page(page)
            .page_size(size)
            .execute();
    return generate_resp(result.to_json());
}