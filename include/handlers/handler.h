#pragma once
#include <string>
#include <boost/json/value.hpp>
#include "http/HttpRequest.h"
#include <sstream>

class Handler {
protected:
    std::map<std::string, std::shared_ptr<Handler>> operators_;
public:
    virtual ~Handler() = default;
    virtual std::string handle(const HttpRequest &request) = 0;
    std::string process(const HttpRequest &req) {
        if (auto it = operators_.find(req.method); it != operators_.end()) {
            return it->second->handle(req);
        }
        return generate_404_response();
    }

    static std::string generate_404_response() {
        return "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: 13\r\n"
               "\r\n"
               "404 Not Found";
    }

    template<typename T>
    static std::string generate_resp(const T &&response) {
        std::stringstream ss;
        ss << "HTTP/1.1 500 ERROR\r\n"
           << "Content-Type: application/json\r\n"
           << "Content-Length: " << boost::json::serialize(response).size() << "\r\n"
           << "\r\n"
           << boost::json::serialize(response);
        return ss.str();
    }

    static std::string generate_error_response(const std::string &response) {
        std::stringstream ss;
        ss << "HTTP/1.1 500 ERROR\r\n"
           << "Content-Type: application/json\r\n"
           << "Content-Length: " << boost::json::serialize(response).size() << "\r\n"
           << "\r\n"
           << boost::json::serialize(response);
        return ss.str();
    }
    static std::string normal_response(boost::json::object &response) {

        std::stringstream ss;
        ss << "HTTP/1.1 200 OK\r\n"
           << "Content-Type: application/json\r\n"
           << "Content-Length: " << boost::json::serialize(response).size() << "\r\n"
           << "\r\n"
           << boost::json::serialize(response);

        return ss.str();
    }

    static std::string normal_rvalue_response(boost::json::object &&response) {

        std::stringstream ss;
        ss << "HTTP/1.1 200 OK\r\n"
           << "Content-Type: application/json\r\n"
           << "Content-Length: " << boost::json::serialize(response).size() << "\r\n"
           << "\r\n"
           << boost::json::serialize(response);

        return ss.str();
    }
    static std::string normal_array_response(boost::json::array &response) {

        std::stringstream ss;
        ss << "HTTP/1.1 200 OK\r\n"
           << "Content-Type: application/json\r\n"
           << "Content-Length: " << boost::json::serialize(response).size() << "\r\n"
           << "\r\n"
           << boost::json::serialize(response);

        return ss.str();
    }
};


