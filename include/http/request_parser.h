//
// Created by huangjun on 2025/2/27.
#pragma once

#include <string>
#include <boost/json.hpp>
#include <map>
#include "http_request.h"
#include <charconv>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>




class RequestParser {
    RequestParser() = default;

private:

// UUID生成辅助函数
   static std::string generate_uuid() {
        boost::uuids::random_generator gen;  // 使用Boost的随机UUID生成器
        boost::uuids::uuid uuid = gen();
        return boost::uuids::to_string(uuid);  // 转换为字符串格式
    }

   static std::string sanitize_filename(const std::string& name) {
        std::filesystem::path p(name);
        return p.filename().string(); // 剥离路径信息
    }

    static bool isMultipartComplete(const std::string& data, const std::string& boundary) {
        size_t start = data.find(boundary);
        size_t end = data.rfind( boundary+"--"); // 标准multipart结尾
        return (start != std::string::npos) && (end == data.size() - boundary.size() - 4);
    }

public:
   static std::string urldecode(const std::string& str) {
       std::ostringstream decoded;
       for (size_t i = 0; i < str.size(); ++i) {
           if (str[i] == '+') {
               decoded << ' ';
           } else if (str[i] == '%' && i + 2 < str.size()) {
               int hex_value;
               std::istringstream hex_stream(str.substr(i + 1, 2));
               if (hex_stream >> std::hex >> hex_value) {
                   decoded << static_cast<char>(hex_value);
                   i += 2;
               }
           } else {
               decoded << str[i];
           }
       }
       return decoded.str();
   }
    static std::optional<http_request> parse(const std::string &raw_request);

    static bool parsePath(http_request &request);

    static bool parseParams(http_request &request);

    static bool parseHeaders(std::istringstream &iss, http_request &request);

    static bool parseBody(std::istringstream &iss, http_request &req);

    static void parse_multipart_form_data(http_request& req, const std::string& boundary);
};