//
// Created by qing on 2025/3/5.
//

#ifndef ORDER_H
#define ORDER_H

#include "model_traits.h"
#include "model.h"

class Order : public Model<Order> {
public:
    Order(int id,
          int order_no,
          int order_qty,
          double unit_price,
          int create_by,
          std::string create_time) : id(id), order_no(order_no), order_qty(order_qty), unit_price(unit_price),
                                     create_by(create_by), create_time(std::move(create_time)) {
        set("id", id);
        set("order_qty", order_qty);
        set("unit_price", unit_price);
        set("order_no", order_no);
        set("create_by", std::move(create_by));
        set<std::string>("create_time", std::move(create_time));


    }

    explicit Order(int id) : id(id),order_no(0) {
        set<int>("id", std::move(id));
        set<int>("order_no", 0);

    }

    Order() = default;

    std::vector<std::pair<MessageLevel, std::string>> verify() override;

    int id;
    int order_no;
    int order_qty;
    double unit_price;
    int create_by;
    std::string create_time;

    static void register_fields();


    [[nodiscard]] boost::json::object to_json() const override {
        return json::object{
                {"id",          id},
                {"order_no",    order_no},
                {"unit_price",  unit_price},
                {"create_by",   create_by},
                {"create_time", create_time}
        };
    }

};


#endif //ORDER_H