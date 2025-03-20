//
// Created by qing on 25-3-6.
//
#include "model/order.h"
#include "model/user.h"

#include "model/model_traits.h"

void Order::register_fields() {
    auto &traits = ModelTraits<Order>::instance();
    traits.set_table_name("orders");
    traits.set_primary_key("id");
    Order::base_register_field(traits, "id", &Order::id);
    Order::base_register_field<int>(traits, "order_no", &Order::order_no);
    Order::base_register_field<std::string>(traits, "create_time", &Order::create_time);
    Order::base_register_field<int>(traits, "order_qty", &Order::order_qty);
    Order::base_register_field<int>(traits, "create_by", &Order::create_by);
    Order::base_register_field<double>(traits, "unit_price", &Order::unit_price);
}

std::vector<std::pair<MessageLevel,std::string>>  Order::verify() {

    User user(this->create_by);

    int count =  user.builder().eq_condition<int>("id").execute_count();
    if (count < 1) {
        std::string errorMsg = std::format("Not find create by user {}", this->create_by).c_str();
        return {std::make_pair(MessageLevel::ERROR,errorMsg)};
    }

    return {};

}





