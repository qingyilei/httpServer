//
// Created by qing on 2025/3/5.
//

#ifndef ORDER_USER_H
#define ORDER_USER_H

#include "model_traits.h"
#include "model.h"

class OrderUser : public Model<OrderUser> {
public:
    OrderUser(
          int order_no,
          int order_qty,
          double unit_price,
          std::string create_name,
          std::string create_time) : order_no(order_no), order_qty(order_qty), unit_price(unit_price),
                                     create_name(create_name), create_time(std::move(create_time)) {
        set("order_qty", order_qty);
        set("unit_price", unit_price);
        set("order_no", order_no);
        set("create_name", std::move(create_name));
        set<std::string>("create_time", std::move(create_time));


    }

    OrderUser() = default;

    std::vector<std::pair<MessageLevel, std::string>> verify() override;

    int order_no;
    int order_qty;
    std::string create_name;
    double unit_price;
    std::string create_time;

    static void register_fields();


    [[nodiscard]] boost::json::object to_json() const override {
        return json::object{
                {"order_no",    order_no},
                {"order_qty",  order_qty},
                {"unit_price",  unit_price},
                {"create_name",   create_name},
                {"create_time", create_time}
        };
    }

};


#endif //ORDER_USER_H