//
// Created by qing on 25-3-11.
//
#include "register/register_center.h"
#include "model/user.h"
void RegisterCenter::register_handler() {
    User::register_fields();
}

RegisterCenter &RegisterCenter::instance() {
    static RegisterCenter instance;
    return instance;
}
