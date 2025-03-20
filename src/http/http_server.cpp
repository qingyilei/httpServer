#include "http/http_server.h"
#include <boost/beast/http.hpp> // 新增Beast库支持
#include <fstream>
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
//
//void HttpServer::accept_connections() {
//    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
//    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
//        if (!ec) {
//            // 设置读取超时
//            socket->set_option(boost::asio::ip::tcp::socket::linger(true, 30)); // 30秒超时
//
//            // 异步读取请求
//            auto buffer = std::make_shared<boost::asio::streambuf>();
//            boost::asio::async_read_until(*socket, *buffer, "\r\n\r\n",
//                                          [this, socket, buffer](const boost::system::error_code &ec, size_t) {
//                                              if (!ec) {
//                                                  std::istream is(buffer.get());
//                                                  std::string req_str(
//                                                          (std::istreambuf_iterator<char>(is)),
//                                                          std::istreambuf_iterator<char>());
//
//                                                  auto content_length = extract_content_length(req_str);
//                                                  auto boundary = extract_boundary(req_str);
//                                                  if (content_length > 0 && !boundary.empty()) {
//                                                      // 读取完整请求体
//                                                      auto buffer1 = std::make_shared<boost::asio::streambuf>();
//
//                                                      boost::asio::async_read(
//                                                              *socket,
//                                                              *buffer1,
//                                                              boost::asio::transfer_exactly(content_length),
//                                                              [this, socket, buffer1, boundary](
//                                                                      const boost::system::error_code &ec, size_t) {
//                                                                  if (!ec) {
//                                                                      // 解析multipart数据
//                                                                      std::istream body_stream(buffer1.get());
//                                                                      std::string body_str{
//                                                                              std::istreambuf_iterator<char>(
//                                                                                      body_stream),
//                                                                              std::istreambuf_iterator<char>()};
//
//                                                                      // 提取文件数据
//                                                                      std::string boundary_str = "--" + boundary;
//                                                                      size_t start = body_str.find(boundary_str) +
//                                                                                     boundary_str.size();
//                                                                      size_t end = body_str.rfind(boundary_str);
//                                                                      std::string file_data = body_str.substr(start,
//                                                                                                              end -
//                                                                                                              start);
//
//                                                                      // 保存文件
//                                                                      try {
//                                                                          std::string filename = "uploaded_file_" + std::to_string(std::time(nullptr)) + ".dat";
//                                                                          std::ofstream outfile(filename, std::ios::binary);
//                                                                          outfile.write(file_data.data(), file_data.size());
//                                                                          outfile.close();
//
//                                                                          // 返回成功响应
//                                                                          std::string response = "HTTP/1.1 200 OK\r\n";
//                                                                          response += "Content-Type: text/plain\r\n";
//                                                                          response += "\r\n";
//                                                                          response += "File uploaded successfully: " + filename;
//                                                                          boost::asio::async_write(*socket,
//                                                                                                   boost::asio::buffer(response),
//                                                                                                   [socket](boost::system::error_code, size_t) {});
//                                                                      } catch (const std::exception& e) {
//                                                                          // 返回500错误
//                                                                          std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n";
//                                                                          error_response += "Content-Type: text/plain\r\n";
//                                                                          error_response += "\r\n";
//                                                                          error_response += "Error saving file: " + std::string(e.what());
//                                                                          boost::asio::async_write(*socket,
//                                                                                                   boost::asio::buffer(error_response),
//                                                                                                   [socket](auto...) {});
//                                                                      }
//                                                                  } else {
//                                                                      // 返回400错误
//                                                                      std::string error_response = "HTTP/1.1 400 Bad Request\r\n";
//                                                                      error_response += "Content-Type: text/plain\r\n";
//                                                                      error_response += "\r\n";
//                                                                      error_response += "Error reading request body";
//                                                                      boost::asio::async_write(*socket,
//                                                                                               boost::asio::buffer(error_response),
//                                                                                               [socket](auto...) {});
//                                                                  }
//                                                              });
//                                                  } else {
//                                                      // 返回400错误
//                                                      std::string error_response = "HTTP/1.1 400 Bad Request\r\n";
//                                                      error_response += "Content-Type: text/plain\r\n";
//                                                      error_response += "\r\n";
//                                                      error_response += "Invalid request format";
//                                                      boost::asio::async_write(*socket,
//                                                                               boost::asio::buffer(error_response),
//                                                                               [socket](auto...) {});
//                                                  }
//                                              } else {
//                                                  // 返回500错误
//                                                  std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n";
//                                                  error_response += "Content-Type: text/plain\r\n";
//                                                  error_response += "\r\n";
//                                                  error_response += "Error reading request";
//                                                  boost::asio::async_write(*socket,
//                                                                           boost::asio::buffer(error_response),
//                                                                           [socket](auto...) {});
//                                              }
//                                          });
//        } else {
//            std::string error_response = "HTTP/1.1 500 Internal Server Error\r\n";
//            error_response += "Content-Type: text/plain\r\n";
//            error_response += "\r\n";
//            error_response += "Error reading request";
//            boost::asio::async_write(*socket,
//                                     boost::asio::buffer(error_response),
//                                     [socket](auto...) {});
//        }
//        accept_connections();
//    });

void HttpServer::accept_connections() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);

    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (ec) {
            handle_error(socket, "Accept failed: " + ec.message());
            return accept_connections();
        }

        // 设置超时控制
        socket->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
        boost::asio::socket_base::linger linger_opt(true, 0);
        socket->set_option(linger_opt);

        // 使用智能指针管理请求上下文
        auto ctx = std::make_shared<RequestContext>();
        ctx->socket = socket;

        // 异步读取请求头
        boost::asio::async_read_until(*socket, ctx->header_buf, "\r\n\r\n",
            [this, ctx](const boost::system::error_code& ec, size_t) {
                handle_header(ctx, ec);
            });

        accept_connections(); // 继续接受新连接
    });
}


// 请求头处理函数
void HttpServer::handle_header(std::shared_ptr<RequestContext> ctx, boost::system::error_code ec) {
    if (ec) {
        return send_response(ctx->socket, 400, "Bad Request: " + ec.message());
    }

    try {
        // 使用Beast解析HTTP头
        ctx->parser = std::make_unique<boost::beast::http::request_parser<boost::beast::http::string_body>>();
        ctx->parser->body_limit(1024 * 1024 * 100); // 限制100MB

        // 解析头信息
        auto buffers = ctx->header_buf.data();
        size_t bytes_used = ctx->parser->put(boost::beast::buffers_prefix(buffers.size(), buffers), ec);

        if(ec) {
            return send_response(ctx->socket, 400, "Parse header failed: " + ec.message());
        }

// 正确消耗已解析的数据
        ctx->header_buf.consume(bytes_used);

// 添加头解析完成检查
        if(ctx->parser->is_done()) {
            return send_response(ctx->socket, 400, "Incomplete headers");
        }        ctx->header_buf.consume(bytes_used);

        // 检查是否multipart上传
        auto& req = ctx->parser->get();
        if (!req.has_content_length() ||
            req.find(boost::beast::http::field::content_type) == req.end() ||
            req[boost::beast::http::field::content_type].find("multipart/form-data") == std::string::npos)
        {
            return send_response(ctx->socket, 415, "Unsupported Media Type");
        }

        // 异步读取请求体
        boost::asio::async_read(*ctx->socket, ctx->body_buf,
            boost::asio::transfer_exactly(ctx->parser->content_length()),
            [this, ctx](const auto& ec, size_t) {
                handle_body(ctx, ec);
            });
    } catch (const std::exception& e) {
        send_response(ctx->socket, 500, "Internal Error: " + std::string(e.what()));
    }
}

// 请求体处理函数
void HttpServer::handle_body(std::shared_ptr<RequestContext> ctx, boost::system::error_code ec) {
    if (ec) {
        return send_response(ctx->socket, 400, "Body Read Error: " + ec.message());
    }

    try {
        // 完成解析请求体
        ctx->parser->put(ctx->body_buf.data());
        ctx->body_buf.consume(ctx->parser->content_length());

        // 提取文件内容
        auto& req = ctx->parser->get();
        if (req.body().empty()) {
            return send_response(ctx->socket, 400, "Empty file content");
        }

        // 生成唯一文件名
        std::string filename = "upload_" +
            boost::uuids::to_string(boost::uuids::random_generator()()) + ".dat";

        // 异步写入文件（使用独立线程池）
        boost::asio::post(file_io_pool_, [ctx, filename]() {
            try {
                std::ofstream file(filename, std::ios::binary);
                file.write(req.body().data(), req.body().size());
                file.close();

                // 返回成功响应
                send_response(ctx->socket, 200, "File saved: " + filename);
            } catch (...) {
                send_response(ctx->socket, 500, "File save failed");
            }
        });
    } catch (const std::exception& e) {
        send_response(ctx->socket, 500, "Processing Error: " + std::string(e.what()));
    }
}

// 通用响应函数
void HttpServer::send_response(
    std::shared_ptr<boost::asio::ip::tcp::socket> socket,
    int status,
    const std::string& message)
{
    boost::beast::http::response<boost::beast::http::string_body> res{
        static_cast<boost::beast::http::status>(status), 11};
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = message;
    res.prepare_payload();

    boost::asio::async_write(*socket, boost::asio::buffer(res.to_string()),
        [socket](boost::system::error_code, size_t) {});
}

void HttpServer::handle_error(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& msg) {
    // 使用通用响应函数发送错误
    send_response(socket, 500,
                  "HTTP/1.1 500 Internal Server Error\r\n"
                  "Content-Type: text/plain\r\n\r\n"
                  "Error: " + msg);
}

