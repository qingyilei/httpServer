
#include "handlers/order/order_handler.h"
#include "handlers/order/order_create_handler.h"
#include "handlers/order/order_query_handler.h"
#include "handlers/order/order_update_handler.h"
#include "handlers/order/order_delete_handler.h"
#include "handlers/order/order_batch_handler.h"
#include "handlers/order/order_user_query_handler.h"
#include "model/order.h"

#include <sstream>


OrderHandler::OrderHandler() {
    operators_.emplace("POST", std::make_shared<OrderCreateHandler>());
    operators_.emplace("GET", std::make_shared<OrderQueryHandler>());
    operators_.emplace("PUT", std::make_shared<OrderUpdateHandler>());
    operators_.emplace("DELETE", std::make_shared<OrderDeleteHandler>());
    operators_.emplace("batchSave", std::make_shared<OrderBatchHandler>());
    operators_.emplace("orderUser", std::make_shared<OrderUserQueryHandler>());


}
OrderHandler::~OrderHandler() {
    if (!operators_.empty()) {
        operators_.clear();
    }
}



