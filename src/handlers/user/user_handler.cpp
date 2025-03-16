
#include "handlers/user/user_handler.h"
#include "handlers/user/user_create_handler.h"
#include "handlers/user/user_query_handler.h"
#include "handlers/user/user_update_handler.h"
#include "handlers/user/user_delete_handler.h"
#include "handlers/user/user_batch_handler.h"
#include "model/user.h"

#include <sstream>


UserHandler::UserHandler() {
    operators_.emplace("POST", std::make_shared<UserCreateHandler>());
    operators_.emplace("GET", std::make_shared<UserQueryHandler>());
    operators_.emplace("PUT", std::make_shared<UserUpdateHandler>());
    operators_.emplace("DELETE", std::make_shared<UserDeleteHandler>());
    operators_.emplace("batchSave",std::make_shared<UserBatchHandler>());
}

UserHandler::~UserHandler() {
    if (!operators_.empty()) {
        operators_.clear();
    }
}


