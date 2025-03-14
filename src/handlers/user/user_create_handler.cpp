#include "handlers/user/user_create_handler.h"
#include <string>
#include "model/user.h"
#include "sql/sql_operator.h"

std::string UserCreateHandler::handle(const HttpRequest &request) {

    int id = request.request_body_field<int>("id");
    User user = User(id,
                     request.request_body_field<std::string>("name"),
                     request.request_body_field<std::string>("email"),
                     request.request_body_field<int>("age"));


//    user.save();
if (id > 0)  {
    SqlOperator<User>::instance().update()
            ->table("user").fields()
            ->clear_field("id").where()
            ->eq<int>("id", user)
            .operator_sql()->save_execute(user);

} else {
    SqlOperator<User>::instance().insert()
            ->table("user").fields()
            ->clear_field("id")
            .clear_fields().field("name")
            .field("email").field("age")
            .operator_sql()
            ->save_execute(user);
}



    return normal_rvalue_response(user.to_json());
}