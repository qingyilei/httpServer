#include "handlers/user/user_query_handler.h"
#include "model/user.h"
#include "sql/sql_operator.h"
#include <sstream>

std::string UserQueryHandler::handle(const HttpRequest &request) {
    auto &params = request.params;
    int id = CommonUtil::get_param(params, "id", 0);
    int age = CommonUtil::get_param(params, "age", 0);
    std::vector<int> ages = CommonUtil::get_all_params<int>(params, "ages");
    int page = CommonUtil::get_param(params, "page", 1);
    int size = CommonUtil::get_param(params, "pageSize", 10);
    std::string name = CommonUtil::get_param(params, "name", std::string(""));
    std::string email = CommonUtil::get_param(params, "email", std::string(""));
    User user(id, name, email, age);

    std::vector<std::pair<std::string, bool>> ids;
    ids.emplace_back("id", true);
        auto result =  SqlOperator<User>::instance()
                .select()
                ->table("user").fields()
                ->where()
                ->like_condition("name",user)
                 .r_like_condition("email",user)
                 .in_condition<int>("age", ages)
                 .page(page,size)
                 ->order_by(ids)
                 .operator_sql()->query_execute();


    //    std::unique_ptr<SqlTable<User>> tableUser =
//    .select()->table("user")
//    .fields()
//    .where()
//    .like_condition("name")
//    .r_like_condition("email")
//    .in_condition<int>("age", ages)
//    .order_by("id", false)
//    .page(page)
//    .page_size(size)
//    .execute();

//    auto result = user.builder()
//            .like_condition("name")
//            .r_like_condition("email")
//            .in_condition<int>("age", ages)
//            .order_by("id", false)
//            .page(page)
//            .page_size(size)
//            .execute();
    return generate_resp(result.to_json());

}