#include <unordered_map>
#include <functional>
#include <string>
#include <boost/json/value.hpp>  // 添加JSON支持
#include "request_parser.h"
class Router {
public:
    using Handler = std::function<std::string(const HttpRequest &)>;

    Router& add_route(const std::string& path, Handler handler) {
        routes_.emplace(path, std::move(handler));
        return *this;
    }

    std::string handle_request(HttpRequest& req) {
        std::string domain = req.domainOperator.first;
        if (auto it = routes_.find(domain); it != routes_.end()) {
            return it->second(req);
        }
        return generate_404_response();
    }

private:
    std::unordered_map<std::string, Handler> routes_;

    static std::string generate_404_response() {
        return "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: 13\r\n"
               "\r\n"
               "404 Not Found";
    }
};


