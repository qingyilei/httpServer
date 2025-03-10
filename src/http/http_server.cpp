#include "http/http_server.h"

HttpServer::HttpServer(unsigned short port, size_t thread_pool_size)
        : acceptor_(io_context_, {boost::asio::ip::tcp::v4(), port}),
          thread_pool_size_(thread_pool_size) {}

void HttpServer::start() {
    accept_connections();

    std::vector<std::thread> threads;
    for (size_t i = 0; i < thread_pool_size_; ++i) {
        threads.emplace_back([this] { io_context_.run(); });
    }

    for (auto &t: threads) {
        if (t.joinable()) t.join();
    }
}

void HttpServer::accept_connections() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            // 异步读取请求
            auto buffer = std::make_shared<boost::asio::streambuf>();
            boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
                                          [this, socket, buffer](const boost::system::error_code &ec, size_t) {
                                              if (!ec) {
                                                  std::istream is(buffer.get());
                                                  std::string req_str(
                                                          (std::istreambuf_iterator<char>(is)),
                                                          std::istreambuf_iterator<char>());
                                                  if (auto opt_req = RequestParser::parse(req_str); opt_req) {
                                                      // 获取实际请求对象
                                                      auto &req = opt_req.value();

                                                      // 路由处理
                                                      std::string response = router_.handle_request(req);

                                                      // 异步发送响应（保持原逻辑）
                                                      boost::asio::async_write(*socket,
                                                                               boost::asio::buffer(response),
                                                                               [socket](boost::system::error_code,
                                                                                        size_t) {});
                                                  } else {
                                                      // 返回400错误等错误处理逻辑
                                                      std::string error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
                                                      boost::asio::async_write(*socket,
                                                                               boost::asio::buffer(error_response),
                                                                               [socket](auto...) {});
                                                  }

                                              }
                                          });
        }
        accept_connections();
    });
}
