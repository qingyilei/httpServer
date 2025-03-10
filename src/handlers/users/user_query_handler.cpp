#include "handlers/user/user_query_handler.h"
#include "model/user.h"
#include "utils/convert_util.h"
#include <sstream>

std::string UserQueryHandler::handle(const HttpRequest &request) {
    auto &params = request.params;
    int id = get_param(params, "id", 0);
    int age = get_param(params, "age", 0);
    std::vector<int> ages = get_all_params<int>(params, "ages");
    int page = get_param(params, "page", 1);
    int size = get_param(params, "pageSize", 10);
    std::string name = get_param(params, "name", std::string(""));
    std::string email = get_param(params, "email", std::string(""));
    User user(id, name, email, age);

    auto result = user.builder()
            .like_condition("name")
            .r_like_condition("email")
            .in_condition<int>("age", ages)
            .order_by("id", false)
            .page(page)
            .page_size(size)
            .execute();
    return generate_resp(result.to_json());
}