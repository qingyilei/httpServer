//
// Created by qing on 25-3-11.
//

#ifndef HTTPSERVER_REGISTER_CENTER_H
#define HTTPSERVER_REGISTER_CENTER_H
#include <coroutine>
class RegisterCenter {
public:
    static RegisterCenter &instance();
    static void register_handler();
private:
    RegisterCenter() = default;
};
#endif //HTTPSERVER_REGISTER_CENTER_H
