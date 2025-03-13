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
        set<int>("id", this->id);
        set<std::string>("create_time", this->create_time);
        set<int>("order_qty", this->order_qty);
        set<double>("unit_price", this->unit_price);
        set<int>("order_no", this->order_no);
        set<int>("create_by", this->create_by);

    }

    explicit Order(int id) : id(id) {
        set<int>("id", this->id);
    }

    Order() = default;

    std::vector<std::pair<MessageLevel,std::string>>  verify() override;

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

//    template<typename T>
//    static void register_field(ModelTraits<Order> &traits, const std::string &name, T Order::* member);
};


#endif //ORDER_H