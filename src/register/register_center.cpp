//
// Created by qing on 25-3-11.
//
#include "register/register_center.h"
#include "model/user.h"
#include "model/order.h"

void RegisterCenter::register_handler() {
    User::register_fields();
    Order::register_fields();
}

RegisterCenter &RegisterCenter::instance() {
    static RegisterCenter instance;
    return instance;
}


