#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include "router.h"

class HttpServer {
public:
    HttpServer(unsigned short port, size_t thread_pool_size = std::thread::hardware_concurrency());
    void start();
    Router& router() { return router_; }

private:
    void accept_connections();

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    Router router_;
    size_t thread_pool_size_;
};
//
// Created by huangjun on 2025/2/27.
//

