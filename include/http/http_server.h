#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include "router.h"
#include <boost/beast/http.hpp> // 新增Beast库支持

class HttpServer {
public:
    // 新增请求上下文结构体
    struct RequestContext {
        std::shared_ptr<boost::asio::ip::tcp::socket> socket;
        boost::asio::streambuf header_buf;
        boost::asio::streambuf body_buf;
        std::unique_ptr<boost::beast::http::request_parser<boost::beast::http::string_body>> parser;
    };

    HttpServer(unsigned short port, size_t thread_pool_size = std::thread::hardware_concurrency());

    void start();

    Router &router() { return router_; }

    void send_response(
            std::shared_ptr<boost::asio::ip::tcp::socket> socket,
            int status,
            const std::string &message);

    void handle_header(std::shared_ptr<RequestContext> ctx, boost::system::error_code ec);

    void handle_body(std::shared_ptr<RequestContext> ctx, boost::system::error_code ec);

    void handle_error(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &msg);

private:
    void accept_connections();

    static std::string extract_boundary(const std::string &req_str) {
        std::istringstream iss(req_str);
        std::string line;
        while (std::getline(iss, line)) {
            // 查找Content-Type头行
            if (line.find("Content-Type: multipart/form-data") != std::string::npos) {
                size_t boundary_pos = line.find("boundary=");
                if (boundary_pos != std::string::npos) {
                    // 提取boundary参数值
                    std::string boundary_str = line.substr(boundary_pos + 9); // "boundary="占9个字符

                    // 去除前导空格和引号
                    boundary_str.erase(0, boundary_str.find_first_not_of(" \t\"'"));

                    // 查找分号或行尾，截取有效值
                    size_t end_pos = boundary_str.find(';');
                    if (end_pos != std::string::npos) {
                        boundary_str = boundary_str.substr(0, end_pos);
                    }

                    // 去除末尾的引号和空格
                    boundary_str.erase(boundary_str.find_last_not_of(" \t\"')") + 1);

                    return boundary_str;
                }
            }
        }
        return ""; // 未找到boundary
    }

    static size_t extract_content_length(const std::string &req_str) {
        size_t content_length = 0;
        std::istringstream iss(req_str);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("Content-Length: ") == 0) {
                try {
                    content_length = std::stoul(line.substr(16));
                } catch (const std::exception &) {
                    content_length = 0; // 解析失败
                }
                break;
            }
        }
        return content_length;
    }

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    Router router_;
    size_t thread_pool_size_;
};
