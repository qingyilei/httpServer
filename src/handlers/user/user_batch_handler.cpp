#include "handlers/user/user_batch_handler.h"
#include <string>
#include "model/user.h"
#include "sql/sql_operator.h"

std::string UserBatchHandler::handle(const http_request &request) {

    int id = request.request_body_field<int>("id");
    User user = User(id,
                     request.request_body_field<std::string>("name"),
                     request.request_body_field<std::string>("email"),
                     request.request_body_field<int>("age"));


    std::vector<User> users;
    for (int i = 0; i < 50; ++i) {
        users.push_back(user);
    }
    int size = SqlOperator<User>::instance().insert()
            ->fields()
            ->clear_field("id")
            .operator_sql()->batch_execute(users);

    return generate_resp<std::string>(std::format("batch size {} success", size));

}