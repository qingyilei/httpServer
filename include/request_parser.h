//
// Created by huangjun on 2025/2/27.
#pragma once

#include <string>
#include <boost/json.hpp>
#include <map>  // 添加头文件以支持std::map
#include "HttpRequest.h"
#include <charconv>
#include <sstream>



class RequestParser {
    RequestParser() = default;

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
    static std::optional<HttpRequest> parse(const std::string &raw_request);

    static bool parsePath(HttpRequest &request);

    static bool parseParams(HttpRequest &request);

    static bool parseHeaders(std::istringstream &iss, HttpRequest &request);

    static bool parseBody(std::istringstream &iss, HttpRequest &req);
};