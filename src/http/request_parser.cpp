
#include "http/request_parser.h"
#include "handlers/user/user_update_handler.h"
#include "handlers/user/user_query_handler.h"
#include "handlers/user/user_delete_handler.h"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iostream>

#include <utility>
#include <stdexcept>

std::optional<http_request> RequestParser::parse(const std::string &raw_request) {
    http_request req;
    std::istringstream iss(raw_request);  // 使用传入的 raw_request 而不是硬编码的字符串
    std::string line;

    // 解析请求行
    if (!std::getline(iss, line)) return std::nullopt;
    std::vector<std::string> parts;
    boost::split(parts, line, boost::is_any_of(" "));
    if (parts.size() < 3) return std::nullopt;  // 请求行应至少包含三个部分：方法、路径和协议版本
    req.method = parts[0];
    req.path = parts[1];

    // 解析Headers和Body
    parseParams(req);
    parsePath(req);
    if (!parseHeaders(iss, req)) return std::nullopt;
    if (!parseBody(iss, req)) return std::nullopt;

    return req;
}

bool RequestParser::parseParams(http_request &request) {
    std::multimap<std::string, std::string> params;

    size_t query_start = request.path.find('?');
    if (query_start == std::string::npos) {
        return false;
    }
    const std::string &query = request.path.substr(query_start + 1);
    size_t start = 0;
    while (start < query.size()) {
        size_t end = query.find('&', start);
        if (end == std::string::npos) end = query.size();

        std::string pair = query.substr(start, end - start);
        size_t eq_pos = pair.find('=');

        std::string key, value;
        if (eq_pos != std::string::npos) {
            key = urldecode(pair.substr(0, eq_pos));
            value = urldecode(pair.substr(eq_pos + 1));
        } else {
            key = urldecode(pair);
        }
        if (!key.empty()) {
            params.emplace(std::move(key), std::move(value));
        }

        start = end + 1;
    }
    request.params = std::move(params);
    return true;
}

bool RequestParser::parsePath(http_request &request) {
    size_t domain_start = 0;
    size_t domain_end = std::string::npos;
    size_t operation_start = std::string::npos;
    size_t operation_end = std::string::npos;
    std::string path = request.path;
    // 跳过开头的所有斜杠
    while (domain_start < path.size() && path[domain_start] == '/') {
        ++domain_start;
    }

    if (domain_start >= path.size()) {
        throw std::invalid_argument("Invalid path: no domain found");
    }

    // 查找业务领域结束位置
    domain_end = path.find('/', domain_start);
    if (domain_end == std::string::npos) {
        domain_end = path.find('?', domain_start);
    }

    if (domain_end == std::string::npos) {
        domain_end = path.size();
    }
    // 处理没有操作部分的情况
    if (domain_end == std::string::npos) {
        throw std::invalid_argument("Invalid path: no operation found");
    }

    // 跳过中间的所有斜杠
    operation_start = domain_end;
    while (operation_start < path.size() && path[operation_start] == '/') {
        ++operation_start;
    }

    // 查找操作结束位置
    operation_end = path.find('/', operation_start);
    if (operation_end == std::string::npos) {
        operation_end = path.find('?', operation_start);
    }
    if (operation_end == std::string::npos) {
        operation_end = path.size();
    }

    request.domainOperator = std::make_pair(
            path.substr(domain_start, domain_end - domain_start),
            path.substr(operation_start, operation_end - operation_start)
    );
    return true;
}


bool RequestParser::parseHeaders(std::istringstream &iss, http_request &req) {
    std::string line;
    while (std::getline(iss, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) {
            return true;  // 空行表示Header部分结束
        }
        // 假设这里解析 header，根据实际需求添加代码
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            boost::algorithm::trim(key);
            boost::algorithm::trim(value);
            req.headers[key] = value;  // 使用headers属性存储键值对
        }
    }
    return false;  // 如果没有找到空行，表示Header解析失败
}

bool RequestParser::parseBody(std::istringstream &iss, http_request &req) {
    std::string body_content;
    std::string line;
    while (std::getline(iss, line)) {
        body_content += line;
    }
    try {
        // 去除首尾空白字符
        boost::algorithm::trim(body_content);
        req.body = body_content.empty() ? boost::json::parse("{}") : boost::json::parse(body_content);
    } catch (const boost::system::system_error &e) {
        // 记录具体错误信息
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
    return true;
}