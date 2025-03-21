#include "http/http_server.h"
#include "handlers/user/user_handler.h"


int main() {
    HttpServer server(8080);

    // 更新后的路由注册（适配新的Handler签名）
    server.router().add_route(
        "user",
        [](const HttpRequest& req) {
            UserHandler userHandler;
            return userHandler.handle(req);  // 修正拼写错误
        });

    server.start();
    return 0;
}