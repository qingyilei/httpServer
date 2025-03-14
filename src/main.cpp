#include "http/http_server.h"
#include "handlers/user/user_handler.h"
#include "register/register_center.h"
#include "handlers/order//order_handler.h"


int main() {
    HttpServer server(8080);
    RegisterCenter::instance().register_handler();
    // 更新后的路由注册（适配新的Handler签名）
    server.router().add_route(
            "user",
            [](const HttpRequest &req) {
                UserHandler userHandler;
                return userHandler.process(req);
            }).add_route("order",
                         [](const HttpRequest &req) {
                             OrderHandler orderHandler;
                             return orderHandler.process(req);
                         });
    server.start();
    return 0;
}