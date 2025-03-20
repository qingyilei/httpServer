//
// Created by qing on 25-3-6.
//
#include "model/order_user.h"
#include "model/model_traits.h"

void OrderUser::register_fields() {
    auto &traits = ModelTraits<OrderUser>::instance();
    traits.set_table_name("orders");
    OrderUser::base_register_field<int>(traits, "order_no", &OrderUser::order_no);
    OrderUser::base_register_field<std::string>(traits, "create_time", &OrderUser::create_time);
    OrderUser::base_register_field<int>(traits, "order_qty", &OrderUser::order_qty);
    OrderUser::base_register_field<std::string>(traits, "create_name", &OrderUser::create_name);
    OrderUser::base_register_field<double>(traits, "unit_price", &OrderUser::unit_price);
}

std::vector<std::pair<MessageLevel, std::string>> OrderUser::verify() {

    return {};

}





