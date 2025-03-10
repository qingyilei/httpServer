
#include "handlers/user/user_handler.h"
#include "handlers/user/user_create_handler.h"
#include "handlers/user/user_query_handler.h"
#include "handlers/user/user_update_handler.h"
#include "handlers/user/user_delete_handler.h"
#include "model/user.h"

#include <sstream>


UserHandler::UserHandler() {
    operators_.emplace("create", std::make_shared<UserCreateHandler>());
    operators_.emplace("query", std::make_shared<UserQueryHandler>());
    operators_.emplace("update", std::make_shared<UserUpdateHandler>());
    operators_.emplace("delete", std::make_shared<UserDeleteHandler>());
}


std::string UserHandler::handle(const HttpRequest &req) {
    User::register_fields();
    // 示例：处理查询参数
    std::string operatorStr = req.domainOperator.second;
    if (auto it = operators_.find(operatorStr); it != operators_.end()) {
        return it->second->handle(req);
    }
    return generate_404_response();
}

UserHandler::~UserHandler() {
    if (!operators_.empty()) {
        operators_.clear();
    }
}


