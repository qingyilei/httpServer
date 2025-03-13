
#include "handlers/order/order_handler.h"
#include "handlers/order/order_create_handler.h"
#include "handlers/order/order_query_handler.h"
#include "handlers/order/order_update_handler.h"
#include "handlers/order/order_delete_handler.h"
#include "model/order.h"

#include <sstream>


OrderHandler::OrderHandler() {
    operators_.emplace("create", std::make_shared<OrderCreateHandler>());
    operators_.emplace("query", std::make_shared<OrderQueryHandler>());
    operators_.emplace("update", std::make_shared<OrderUpdateHandler>());
    operators_.emplace("delete", std::make_shared<OrderDeleteHandler>());
}


std::string OrderHandler::handle(const HttpRequest &req) {

    // 示例：处理查询参数
    std::string operatorStr = req.domainOperator.second;
    if (auto it = operators_.find(operatorStr); it != operators_.end()) {
        return it->second->handle(req);
    }
    return generate_404_response();
}

OrderHandler::~OrderHandler() {
    if (!operators_.empty()) {
        operators_.clear();
    }
}


