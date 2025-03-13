//
// Created by huangjun on 2025/3/1.
//

// 添加头文件以支持std::map

#ifndef HTTP_SERVER_HTTPREQUEST_H
#define HTTP_SERVER_HTTPREQUEST_H
#pragma once

#include <string>
#include <boost/json.hpp>
#include <map>

template<typename T>
struct always_false : std::false_type {
};



struct HttpRequest {
    HttpRequest() = default;

    std::string method;
    std::string path;
    // first:domain ,second:operator
    std::pair<std::string, std::string> domainOperator;
    std::multimap<std::string, std::string> params;
    boost::json::value body;
    std::map<std::string, std::string> headers;  // 添加headers属性

    // 获取 JSON 字段（模板方法）
    template<typename T>
    T request_body_field(const std::string &key, T default_val = T()) const {
        if (!body.is_object()) return default_val;
        auto it = body.as_object().find(key);
        if (it == body.as_object().end()) return default_val;

        if constexpr (std::is_same_v<T, std::string>) {
            if (it->value().is_string()) {
                return it->value().as_string().c_str(); // 移除参数
            }// 注意：Boost 1.75+ 需.c_str()
            return default_val;
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(it->value().as_int64());
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(it->value().as_double());
        } else {
            static_assert(always_false<T>::value, "Unsupported JSON field type");
        }
    }
};


#endif //HTTP_SERVER_HTTPREQUEST_H
